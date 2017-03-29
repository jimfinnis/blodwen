--
-- Parse register descriptions to produce both a set of constants
-- and a register table
--

import System.Console.GetOpt
import System.Environment
import Text.Regex
import Numeric (readSigned, readFloat)
import Text.Parsec hiding ((<|>))
import Text.Parsec.String

-- we're going to use haskell-style comments

import Text.Parsec.Language(haskellDef)
import qualified Text.Parsec.Token as P
import Control.Applicative hiding(many)
import Data.List
import Debug.Trace

-- create a lexer which knows how to skip haskell-style comments

lexer = P.makeTokenParser haskellDef

-- create parsers

identifier = P.identifier lexer
integer = P.integer lexer
hfloat = P.float lexer
naturalOrFloat = P.naturalOrFloat lexer
braces = P.braces lexer
symbol = P.symbol lexer
stringLiteral = P.stringLiteral lexer
whiteSpace = P.whiteSpace lexer

-- a preamble we use at the start of all output files

autopreamble = "/**\n * \\file\n * Code generated automatically by regparse.hs - DO NOT MODIFY.\n */\n\n"


-- a register entry

data Register = Reg {
    mName :: String,
    mBytes :: Integer,
    mWritable :: Bool,
    mDesc :: String,
    mMin :: Double, -- if -1, the value is not mapped from float to int
    mMax :: Double  -- if -1, the value is not mapped from float to int
} deriving(Show)

data Block = Block {
    mPrefix :: String,
    mRegs   :: [Register],
    mNoCommon :: Bool
}

    
float = do
    t <- optionMaybe (char '-')
    let sign = case t of
                (Just x) -> (-1)
                Nothing -> 1
    tt <- naturalOrFloat
    let f = case tt of
                (Left x) -> fromIntegral x
                (Right x) -> x
    return (f*sign)

range = do
    (symbol "range")
    mn <- float
    mx <- float
    return (mn,mx)
    
noRange = do
    (symbol "unmapped")
    return ((-1),(-1))

-- parser, produces [Register]

register = do
    ident <- identifier
    t <- optionMaybe (symbol "writable")
    let writable = case t of
                    (Just x) -> True
                    Nothing -> False
    bytes <- integer
    r <- range <|> noRange
    desc <- stringLiteral
    return (Reg ident bytes writable desc (fst r) (snd r))
    
-- parser, produces Block - i.e. a named block of registers

block = do
    ident <- (option "" identifier)
    t <- optionMaybe (symbol "nocommon")
    let nocom = case t of
                    (Just _) -> True
                    Nothing -> False
    regs <- (braces (many register))
    return (Block ident regs nocom)
    
-- the actual parser, which parses many blocks

blockParser = do
    whiteSpace
    many block
    
parseBlocks input = parse blockParser "unknown" input

-- output a list of registers, getting the strings for each one
-- by mapping (index,register) onto a function. The (index,register)
-- entries are produced by mapping the register list onto a function
-- which associates each one with an index number. The starting
-- index is passed in as well as the function.

outputRegs base f lst = intercalate "\n" $ map f (_number lstr)
                        where
                            lstr = reverse lst
                            _number :: [a] -> [(a,Int)]
                            _number (x:[]) = [(x,base)]
                            _number (x:xs) =  q++[(x,n)]
                                                where
                                                    q = _number xs
                                                    n = 1 + (snd $ last q)

-- output a #define line for a register, given the (number,register) pair

outputNumberedRegH (x,n) = "#define " ++ (mName x) ++ "\t" ++ (show n) ++ " \t//" ++ (mDesc x)

-- output a {regBytes+writable,mi,max} entry for a register, given the (number,register) pair
outputNumberedRegC (x,n) = "\t{" ++ show (sizeAndWritable x) 
                                 ++ ", " ++ (show $ mMin x) 
                                 ++ ", " ++ (show $ mMax x) 
                                 ++ "}, \t// " ++ (mName x) ++ ": (" ++ (show n) ++ ")  "++(mDesc x)
                                where sizeAndWritable x = case (mWritable x) of
                                                    True -> 64 + (mBytes x)
                                                    False -> mBytes x


scoreReg = mkRegex "\\_"

-- output a line describing the register suitable for a latex table

outputNumberedRegLatex (x,n) = intercalate " & " [
                                    show n,
                                    doname (mName x),
                                    show (mBytes x),
                                    if (mWritable x) then "Y" else "",
                                    floatRange (mMin x) (mMax x),
                                    mDesc x ] ++ "\\\\ \\hline"
                                    where
                                        floatRange a b = if a==b then "unmapped"
                                                                 else "[" ++ show a ++ "," ++ show b ++ "]"
                                        doname s = subRegex scoreReg s "\\_"


-- output a block of registers, given the number of items in the common block and the block itself.
-- This will set the base of the indices to the number of items in the common block if this
-- block is not itself the common block. That way, the common block is effectively "prepended" to
-- all the blocks (because it's common to all of them.)

outputBlockC :: Int -> Block -> String
outputBlockC cbs (Block name blocks _) = "extern MAYBEPROGMEM Register registerTable_" ++ name ++ "[]={\n" ++
                            (outputRegs 0 outputNumberedRegC blocks) ++ "\n};\n"
                            
-- handy method to modify a register, producing a copy with the name prepended by REG_name, where name
-- the block name

prefix name (Reg n bytes writable desc a b) = (Reg ("REG"++name++"_"++n) bytes writable desc a b)

-- output a block of registers to an include file, given the number of items in the common block and the block itself.
-- This will set the base of the indices to the number of items in the common block if this
-- block is not itself the common block. That way, the common block is effectively "prepended" to
-- all the blocks (because it's common to all of them.)

outputBlockH :: Int -> Block -> String
outputBlockH cbs b@(Block name blocks _) = (outputRegs base outputNumberedRegH $ map (prefix name) blocks)
                                        ++ "\n" ++ outSize ++ "\n\n" ++ decl ++ "\n\n"
                                where
                                    -- provide a shorthand for "if commonblock then t else f"
                                    ifCommon t f = if isCommonBlock b then t else f

                                    -- constants start at zero for common block or nocommon blocks, otherwise the size
                                    -- of the common block is used
                                    
                                    base = if isCommonBlock b || mNoCommon b then 0 else cbs

                                    -- we output the size too, plus the size of the common block
                                    -- (if not the common block)

                                    outSize = ifCommon "" 
                                        ("#define NUMREGS_"++name++" "++(show $ base+length blocks))
                                                                 
                                    -- and the register table extern declaration
                                    
                                    decl = ifCommon "" ("extern MAYBEPROGMEM Register registerTable_"++name++"[];")

outputBlockLatex :: Int -> Block -> String
outputBlockLatex cbs b@(Block name blocks _) =
    "\\begin{tabular}{|p{0.2in}|p{2.7in}|p{0.1in}|p{0.1in}|p{1in}|p{1.5in}|}\\hline\n" ++
    "\\textbf{ID} & \\textbf{Symbol} & \\textbf{n} & \\textbf{W} & \\textbf{Mapping} & \\textbf{Description}  \\\\ \\hline \n"++
    (outputRegs base outputNumberedRegLatex $ map (prefix name) blocks) ++
    "\n\\end{tabular}\n\n"
        where
            base = if isCommonBlock b || mNoCommon b then 0 else cbs




-- given a function and a list of things, produce a string consisting of the autopreamble followed
-- by an intercalation of the mapping of that function onto the list with return characters

outputBlockList fn xs = intercalate "\n" (map fn xs)

-- is the given block the common block (i.e. does it have a null name?)

isCommonBlock :: Block -> Bool
isCommonBlock (Block prefix _ _) = prefix == ""


-- prepend the common block to all blocks which are NOT the common block, unless
-- they are marked as 'nocommon'

prependCommonToBlocks :: [Block] -> [Block]
prependCommonToBlocks qs = map (prepend cb) qs
                            where
                                prepend (Block x xs _) b@(Block y ys nocom) =
                                    if (isCommonBlock b || nocom) then b
                                                         else (Block y (xs++ys) False)
                                cb = head $ filter (isCommonBlock) qs
                                
    
appendTerminatorToBlocks :: [Block] -> [Block]
appendTerminatorToBlocks xs = map _appendTerm xs
                                where
                                    _appendTerm:: Block -> Block
                                    _appendTerm b@(Block n ys nocom) = 
                                        if not $ isCommonBlock b then (Block n
                                                                        (ys++[(Reg "TERMINATOR" (32) False "" (-1) (-1))])
                                                                          nocom)
                                                                 else (Block n ys nocom)


geninc x = "#include \"" ++ x ++ "\"\n"

cpreamble=autopreamble ++ geninc "regs.h"
hpreamble=autopreamble ++ geninc "regs.h"

main :: IO()
main = do
    args <- getArgs
    x <- readFile (args!!0)
    let q = parseBlocks x
    case q of
        (Right lst) -> do
                            -- get the size of the common block

                            let commonBlockSize = length $ mRegs $ head $ filter (isCommonBlock) lst

                            -- write the H file to regsauto.h

                            writeFile "regsauto.h"   $ 
                                hpreamble ++ (outputBlockList (outputBlockH commonBlockSize) lst)

                            -- write the C file to regsauto.cpp, but don't do the common block. For each block,
                            -- prepend the common block to it.

                            writeFile "regsauto.cpp" $ cpreamble ++ (outputBlockList (outputBlockC commonBlockSize) $ 
                                    filter (\x -> not $ isCommonBlock x)
                                    (prependCommonToBlocks . appendTerminatorToBlocks $ lst))
                                    
                            writeFile "regs.tex" $ outputBlockList (outputBlockLatex commonBlockSize) lst
                                    

        (Left err)  -> putStrLn $ show err



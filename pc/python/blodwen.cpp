/**
 * \file
 * This file contains the binding code needed to create a Python
 * wrapper module for the Blodwen library using pybind11.
 */

#include <pybind11/pybind11.h>
#include "../rover.h"
#include "../comms.h"

namespace py = pybind11;
using namespace pybind11::literals;

// kluge to avoid ending up with undefined symbols associated with header-defined attributes
const StatusFlag SerialComms::CONNECTED;
const StatusFlag SerialComms::ERROR;
const StatusFlag SerialComms::TIMEOUT;
const StatusFlag SerialComms::CONNECTING;
const StatusFlag SerialComms::PROTOCOL_ERROR;
const StatusFlag SerialComms::ERRORFLAGS;


PYBIND11_MODULE(blodwen, m) {
    m.doc() = "Blodwen Rover python wrapper";
    m.attr("DRIVE") = DRIVE;
    m.attr("STEER") = STEER;
    m.attr("LIFT") = LIFT;

    // TODO: not really needed by the wrapper...
//    py::class_<Simulator>(m, "Simulator");
//    py::class_<RoverSimulator, Simulator>(m, "RoverSimulator");
//    py::class_<MotorSim>(m, "MotorSim");

    py::class_<Motor>(m, "Motor")
        .def("getRequired", &Motor::getRequired)
        ;
    py::class_<DriveMotor, Motor>(m, "DriveMotor")
        .def(py::init<SlaveDevice *>())
        .def("sendParams", &DriveMotor::sendParams)
        .def("resetOdometer", &DriveMotor::resetOdometer)
        .def("getParams", &DriveMotor::getParams, py::return_value_policy::reference_internal)
        .def("setRequired", &DriveMotor::setRequired, "speed"_a)
        ;
    py::class_<SteerMotor, Motor>(m, "SteerMotor")
        .def(py::init<SlaveDevice *>())
        .def("sendParams", &SteerMotor::sendParams)
        .def("getParams", &SteerMotor::getParams, py::return_value_policy::reference_internal)
        .def("getPosParams", &SteerMotor::getPosParams, py::return_value_policy::reference_internal)
        .def("setRequired", &SteerMotor::setRequired, "pos"_a)
        ;
    py::class_<LiftMotor, Motor>(m, "LiftMotor")
        .def(py::init<SlaveDevice *, int>())
        .def("sendParams", &LiftMotor::sendParams)
        .def("getParams", &LiftMotor::getParams, py::return_value_policy::reference_internal)
        .def("getPosParams", &LiftMotor::getPosParams, py::return_value_policy::reference_internal)
        .def("setRequired", &LiftMotor::setRequired, "pos"_a)
        .def_readwrite("wheelNumber", &LiftMotor::wheelNumber)
        ;

    py::class_<MasterData>(m, "MasterData")
        .def(py::init<SlaveDevice *>())
        .def_readwrite("exceptionType", &MasterData::exceptionType)
        .def_readwrite("exceptionSlave", &MasterData::exceptionSlave)
        .def_readwrite("exceptionMotor", &MasterData::exceptionMotor)
        .def("init", &MasterData::init)
        .def("update", &MasterData::update)
        ;

    py::class_<MotorParams>(m, "MotorParams")
        .def(py::init())
        .def_readwrite("pGain", &MotorParams::pGain)
        .def_readwrite("iGain", &MotorParams::iGain)
        .def_readwrite("iGain", &MotorParams::dGain)
        .def_readwrite("iCap", &MotorParams::iCap)
        .def_readwrite("iDecay", &MotorParams::iDecay)
        .def_readwrite("overCurrentThresh", &MotorParams::overCurrentThresh)
        .def_readwrite("stallCheck", &MotorParams::stallCheck)
        .def_readwrite("deadZone", &MotorParams::deadZone)
        .def("reset", &MotorParams::reset)
        ;
    py::class_<PosMotorParams, MotorParams>(m, "PosMotorParams")
        .def_readwrite("calibMin", &PosMotorParams::calibMin)
        .def_readwrite("calibMax", &PosMotorParams::calibMax)
        .def("reset", &PosMotorParams::reset)
        ;

    py::class_<MotorDriverData>(m, "MotorDriverData")
        .def_readwrite("timer", &MotorDriverData::timer)
        .def_readwrite("interval", &MotorDriverData::interval)
        .def_readwrite("status", &MotorDriverData::status)
        .def_readwrite("exceptionID", &MotorDriverData::exceptionID)
        .def_readwrite("exceptionType", &MotorDriverData::exceptionType)
        ;
    py::class_<DriveSteerMotorDriverData, MotorDriverData>(m, "DriveSteerMotorDriverData")
        .def(py::init<SlaveDevice *>())
        .def_readwrite("steer", &DriveSteerMotorDriverData::steer)
        .def_readwrite("drive", &DriveSteerMotorDriverData::drive)
        .def_readwrite("chassis", &DriveSteerMotorDriverData::chassis)
        .def("init", &DriveSteerMotorDriverData::init)
        .def("update", &DriveSteerMotorDriverData::update)
        ;

    py::class_<LiftMotorDriverData, MotorDriverData>(m, "LiftMotorDriverData")
        .def(py::init<SlaveDevice *>())
//        .def_readwrite("data", &LiftMotorDriverData::data) // FIXME: pybind doesn't play well with c-style arrays
        .def("init", &LiftMotorDriverData::init)
        .def("update", &LiftMotorDriverData::update)
        ;

    py::class_<MotorData>(m, "MotorData")
        .def_readwrite("error", &MotorData::error)
        .def_readwrite("errorIntegral", &MotorData::errorIntegral)
        .def_readwrite("errorDeriv", &MotorData::errorDeriv)
        .def_readwrite("control", &MotorData::control)
        .def_readwrite("intervalCtrl", &MotorData::intervalCtrl)
        .def_readwrite("current", &MotorData::current)
        .def_readwrite("actual", &MotorData::actual)
        .def_readwrite("exceptionType", &MotorData::exceptionType)
        ;
    py::class_<SteerMotorData, MotorData>(m, "SteerMotorData"); // OK
    py::class_<DriveMotorData, MotorData>(m, "DriveMotorData")
        .def_readwrite("odometer", &DriveMotorData::odometer)
        ;
    py::class_<LiftMotorData, MotorData>(m, "LiftMotorData"); // OK

    py::class_<Register>(m, "Register")
        .def_readwrite("sizeAndFlags", &Register::sizeAndFlags)
        .def_readwrite("minval", &Register::minval)
        .def_readwrite("maxval", &Register::maxval)
        .def("getSize", &Register::getSize)
        .def("writable", &Register::writable)
        .def("isInRange", &Register::isInRange, "v"_a) // TODO: const?
        .def("unmap", &Register::unmap, "i"_a) // TODO: const?
        .def("map", &Register::map, "v"_a) // TODO: const?
        ;

    py::class_<WheelPair>(m, "WheelPair")
        .def(py::init())
        .def("resetExceptions", &WheelPair::resetExceptions)
        .def("getChassisValue", &WheelPair::getChassisValue)
        .def("init", &WheelPair::init, "protocol"_a, "pair"_a)
        .def("setReadSets", &WheelPair::setReadSets)
        .def("update", &WheelPair::update)
        .def("getDevice", &WheelPair::getDevice, "n"_a, py::return_value_policy::reference_internal)  // TODO: double check return (devs are not pointers...)
        .def("getMotor", &WheelPair::getMotor, "n"_a, "type"_a, py::return_value_policy::reference_internal)
        .def("getMotorData", &WheelPair::getMotorData, "n"_a, "type"_a, py::return_value_policy::reference_internal)
        .def("getDriveData", &WheelPair::getDriveData, "n"_a, py::return_value_policy::reference_internal)
        .def("getSteerData", &WheelPair::getSteerData, "n"_a, py::return_value_policy::reference_internal)
        .def("getLiftData", &WheelPair::getLiftData, "n"_a, py::return_value_policy::reference_internal)
        .def("getDrive", &WheelPair::getDrive, "n"_a, py::return_value_policy::reference_internal)
        .def("getSteer", &WheelPair::getSteer, "n"_a, py::return_value_policy::reference_internal)
        .def("getLift", &WheelPair::getLift, "n"_a, py::return_value_policy::reference_internal)
        ;

    // Rover singleton requires some extra accommodations just to be on the safe side at teardown.
    // TODO: multiple "instantiations" still yields Python objects with different identities ("Rover() is not Rover())...
    py::class_<Rover, std::unique_ptr<Rover, py::nodelete>>(m, "Rover")
        .def(py::init([](){
            return std::unique_ptr<Rover, py::nodelete>(Rover::getInstance());
        }))
        .def_readwrite("legCollisionChecksEnabled", &Rover::legCollisionChecksEnabled)
        .def("getPairIdx", &Rover::getPairIdx, "wheelNumber"_a)
        .def("getWheelIdx", &Rover::getWheelIdx, "wheelnumber"_a)
        .def("isValid", &Rover::isValid)
        .def_readwrite("comms", &Rover::comms)
        .def("init", &Rover::init, "port"_a, "pp"_a=7)
        .def_static("getMotorTypeName", &Rover::getMotorTypeName) // TODO: OK? maybe copy policy?
        .def("resetExceptions", &Rover::resetExceptions)
        .def("update", &Rover::update)
        .def("getPair", &Rover::getPair, "n"_a, py::return_value_policy::reference_internal)
        .def("getMotor", &Rover::getMotor, "w"_a, "t"_a, py::return_value_policy::reference_internal)
        .def("getMotorData", &Rover::getMotorData, "w"_a, "t"_a, py::return_value_policy::reference_internal)
        .def("getDriveData", &Rover::getDriveData, "n"_a, py::return_value_policy::reference_internal)
        .def("getSteerData", &Rover::getSteerData, "n"_a, py::return_value_policy::reference_internal)
        .def("getLiftData", &Rover::getLiftData, "n"_a, py::return_value_policy::reference_internal)
        .def("getDrive", &Rover::getDrive, "n"_a, py::return_value_policy::reference_internal)
        .def("getSteer", &Rover::getSteer, "n"_a, py::return_value_policy::reference_internal)
        .def("getLift", &Rover::getLift, "n"_a, py::return_value_policy::reference_internal)
        .def("getMasterData", &Rover::getMasterData, py::return_value_policy::reference_internal)
        .def("calibrate", &Rover::calibrate)
        ;

    py::class_<SlaveProtocol>(m, "SlaveProtocol")
        .def(py::init())
        .def_readwrite("comms", &SlaveProtocol::comms)
        .def("init", &SlaveProtocol::init, "c"_a)
        .def("start", &SlaveProtocol::start, "id"_a, "cmd"_a)
        .def("add", &SlaveProtocol::add, "ptr"_a, "size"_a)
        .def("addByte", &SlaveProtocol::addByte, "b"_a)
        .def("send", &SlaveProtocol::send)
        .def("readBlock", &SlaveProtocol::readBlock, "ptr"_a, "size"_a)
        ;

    py::class_<SlaveDevice>(m, "SlaveDevice")
        .def(py::init())
        .def("getAddr", &SlaveDevice::getAddr)
        .def("init", &SlaveDevice::init, "_p"_a, "table"_a, "id"_a, py::return_value_policy::reference)  // TODO: not entirely sure about policy
        .def("startWrites", &SlaveDevice::startWrites)
        .def("endWrites", &SlaveDevice::endWrites)
        .def("writeInt", &SlaveDevice::writeInt, "reg"_a, "val"_a)
        .def("writeFloat", &SlaveDevice::writeFloat, "r"_a, "v"_a)
        .def("resetExceptions", &SlaveDevice::resetExceptions)
//        .def("setReadSet", &SlaveDevice::setReadSet) // FIXME: Can't deal with va_args
        .def("readRegs", &SlaveDevice::readRegs, "set"_a)
        .def("getRegInt", &SlaveDevice::getRegInt, "n"_a)
        .def("getRegFloat", &SlaveDevice::getRegFloat, "n"_a)
        .def("isConnected", &SlaveDevice::isConnected)
        ;

    py::class_<StatusListener>(m, "StatusListener")
        .def("onMessage", &StatusListener::onMessage, "str"_a)
        .def("onStatusChange", &StatusListener::onStatusChange, "flags"_a)
        ;

    py::class_<StatusObservable>(m, "StatusObservable")
        .def(py::init())
        .def("setStatusListener", &StatusObservable::setStatusListener, "l"_a)
        .def("getStatus", &StatusObservable::getStatus)
//        .def("notifyMessage", &StatusObservable::notifyMessage) // FIXME: va_args...
        .def("setStatus", &StatusObservable::setStatus, "set"_a, "clr"_a=0)
        .def("clrStatus", &StatusObservable::clrStatus, "clr"_a)
        ;
    py::class_<SerialComms, StatusObservable>(m, "SerialComms")
        .def(py::init())
        .def_readonly_static("CONNECTED", &SerialComms::CONNECTED)
        .def_readonly_static("ERROR", &SerialComms::ERROR)
        .def_readonly_static("TIMEOUT", &SerialComms::TIMEOUT)
        .def_readonly_static("CONNECTING", &SerialComms::CONNECTING)
        .def_readonly_static("PROTOCOL_ERROR", &SerialComms::PROTOCOL_ERROR)
        .def_readonly_static("ERRORFLAGS", &SerialComms::ERRORFLAGS)
        .def("isError", &SerialComms::isError)
        .def("isSim", &SerialComms::isSim)
        .def("tickSim", &SerialComms::tickSim)
        .def("pollSim", &SerialComms::pollSim)
        .def("simConnect", &SerialComms::simConnect, "s"_a)
        .def("connect", &SerialComms::connect, "dev"_a, "baudRate"_a)
        .def("setTimeout", &SerialComms::setTimeout, "sec"_a, "usec"_a)
        .def("disconnect", &SerialComms::disconnect)
        .def("write", &SerialComms::write, "s"_a, "ct"_a)
        .def("clearTimeout", &SerialComms::clearTimeout)
        .def("isTimeout", &SerialComms::isTimeout)
        .def("read", &SerialComms::read, "buf"_a, "ct"_a)
        .def("isReady", &SerialComms::isReady)
        .def("readLine", &SerialComms::readLine, "buf"_a, "maxlen"_a)
        ;

    py::register_exception<RoverException>(m, "RoverException");
    py::register_exception<ConstraintException>(m, "ConstraintException");
    py::register_exception<SlaveException>(m, "SlaveException");
}

/****************************************************************************
** Meta object code from reading C++ file 'animation_controller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../source/gui/animation/animation_controller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'animation_controller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN19AnimationControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto AnimationController::qt_create_metaobjectdata<qt_meta_tag_ZN19AnimationControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AnimationController",
        "stepStarted",
        "",
        "AnimationStep",
        "step",
        "stepProgress",
        "progress",
        "stepCompleted",
        "queueSizeChanged",
        "size",
        "queueEmpty",
        "playbackStarted",
        "playbackPaused",
        "playbackResumed",
        "playbackStopped",
        "stepIndexChanged",
        "currentIndex",
        "totalSteps",
        "seekCompleted",
        "stepIndex",
        "stateRestored",
        "AnimationState",
        "state",
        "totalStepsChanged",
        "frameUpdate",
        "deltaTime",
        "requestRepaint",
        "onFrameUpdate",
        "processNextStep",
        "onStepTimeout"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stepStarted'
        QtMocHelpers::SignalData<void(const AnimationStep &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'stepProgress'
        QtMocHelpers::SignalData<void(const AnimationStep &, double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Double, 6 },
        }}),
        // Signal 'stepCompleted'
        QtMocHelpers::SignalData<void(const AnimationStep &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'queueSizeChanged'
        QtMocHelpers::SignalData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Signal 'queueEmpty'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playbackStarted'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playbackPaused'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playbackResumed'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playbackStopped'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stepIndexChanged'
        QtMocHelpers::SignalData<void(int, int)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 16 }, { QMetaType::Int, 17 },
        }}),
        // Signal 'seekCompleted'
        QtMocHelpers::SignalData<void(int)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Signal 'stateRestored'
        QtMocHelpers::SignalData<void(const AnimationState &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Signal 'totalStepsChanged'
        QtMocHelpers::SignalData<void(int)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 17 },
        }}),
        // Signal 'frameUpdate'
        QtMocHelpers::SignalData<void(double, double)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 25 }, { QMetaType::Double, 5 },
        }}),
        // Signal 'requestRepaint'
        QtMocHelpers::SignalData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onFrameUpdate'
        QtMocHelpers::SlotData<void(double)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 25 },
        }}),
        // Slot 'processNextStep'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStepTimeout'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AnimationController, qt_meta_tag_ZN19AnimationControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AnimationController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AnimationControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AnimationControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19AnimationControllerE_t>.metaTypes,
    nullptr
} };

void AnimationController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AnimationController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stepStarted((*reinterpret_cast<std::add_pointer_t<AnimationStep>>(_a[1]))); break;
        case 1: _t->stepProgress((*reinterpret_cast<std::add_pointer_t<AnimationStep>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 2: _t->stepCompleted((*reinterpret_cast<std::add_pointer_t<AnimationStep>>(_a[1]))); break;
        case 3: _t->queueSizeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->queueEmpty(); break;
        case 5: _t->playbackStarted(); break;
        case 6: _t->playbackPaused(); break;
        case 7: _t->playbackResumed(); break;
        case 8: _t->playbackStopped(); break;
        case 9: _t->stepIndexChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 10: _t->seekCompleted((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->stateRestored((*reinterpret_cast<std::add_pointer_t<AnimationState>>(_a[1]))); break;
        case 12: _t->totalStepsChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->frameUpdate((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 14: _t->requestRepaint(); break;
        case 15: _t->onFrameUpdate((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 16: _t->processNextStep(); break;
        case 17: _t->onStepTimeout(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(const AnimationStep & )>(_a, &AnimationController::stepStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(const AnimationStep & , double )>(_a, &AnimationController::stepProgress, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(const AnimationStep & )>(_a, &AnimationController::stepCompleted, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(int )>(_a, &AnimationController::queueSizeChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)()>(_a, &AnimationController::queueEmpty, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)()>(_a, &AnimationController::playbackStarted, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)()>(_a, &AnimationController::playbackPaused, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)()>(_a, &AnimationController::playbackResumed, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)()>(_a, &AnimationController::playbackStopped, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(int , int )>(_a, &AnimationController::stepIndexChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(int )>(_a, &AnimationController::seekCompleted, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(const AnimationState & )>(_a, &AnimationController::stateRestored, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(int )>(_a, &AnimationController::totalStepsChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)(double , double )>(_a, &AnimationController::frameUpdate, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationController::*)()>(_a, &AnimationController::requestRepaint, 14))
            return;
    }
}

const QMetaObject *AnimationController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AnimationController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AnimationControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AnimationController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void AnimationController::stepStarted(const AnimationStep & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AnimationController::stepProgress(const AnimationStep & _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void AnimationController::stepCompleted(const AnimationStep & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AnimationController::queueSizeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void AnimationController::queueEmpty()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void AnimationController::playbackStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void AnimationController::playbackPaused()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void AnimationController::playbackResumed()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void AnimationController::playbackStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void AnimationController::stepIndexChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}

// SIGNAL 10
void AnimationController::seekCompleted(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}

// SIGNAL 11
void AnimationController::stateRestored(const AnimationState & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void AnimationController::totalStepsChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void AnimationController::frameUpdate(double _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1, _t2);
}

// SIGNAL 14
void AnimationController::requestRepaint()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}
QT_WARNING_POP

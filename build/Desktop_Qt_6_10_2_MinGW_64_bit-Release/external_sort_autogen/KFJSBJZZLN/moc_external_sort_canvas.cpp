/****************************************************************************
** Meta object code from reading C++ file 'external_sort_canvas.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../source/gui/visualization/external_sort_canvas.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'external_sort_canvas.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15AnimationEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto AnimationEngine::qt_create_metaobjectdata<qt_meta_tag_ZN15AnimationEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AnimationEngine",
        "frameUpdate",
        "",
        "deltaTime",
        "AggregatedStep",
        "step",
        "progress",
        "stepStarted",
        "stepCompleted",
        "stepIndexChanged",
        "current",
        "total",
        "queueSizeChanged",
        "size",
        "queueEmpty",
        "playbackStarted",
        "playbackStopped",
        "onTimerTick"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'frameUpdate'
        QtMocHelpers::SignalData<void(double, const AggregatedStep &, double)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 }, { 0x80000000 | 4, 5 }, { QMetaType::Double, 6 },
        }}),
        // Signal 'stepStarted'
        QtMocHelpers::SignalData<void(const AggregatedStep &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Signal 'stepCompleted'
        QtMocHelpers::SignalData<void(const AggregatedStep &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Signal 'stepIndexChanged'
        QtMocHelpers::SignalData<void(int, int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 10 }, { QMetaType::Int, 11 },
        }}),
        // Signal 'queueSizeChanged'
        QtMocHelpers::SignalData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Signal 'queueEmpty'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playbackStarted'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'playbackStopped'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onTimerTick'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AnimationEngine, qt_meta_tag_ZN15AnimationEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AnimationEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AnimationEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AnimationEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15AnimationEngineE_t>.metaTypes,
    nullptr
} };

void AnimationEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AnimationEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->frameUpdate((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<AggregatedStep>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 1: _t->stepStarted((*reinterpret_cast<std::add_pointer_t<AggregatedStep>>(_a[1]))); break;
        case 2: _t->stepCompleted((*reinterpret_cast<std::add_pointer_t<AggregatedStep>>(_a[1]))); break;
        case 3: _t->stepIndexChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->queueSizeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->queueEmpty(); break;
        case 6: _t->playbackStarted(); break;
        case 7: _t->playbackStopped(); break;
        case 8: _t->onTimerTick(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)(double , const AggregatedStep & , double )>(_a, &AnimationEngine::frameUpdate, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)(const AggregatedStep & )>(_a, &AnimationEngine::stepStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)(const AggregatedStep & )>(_a, &AnimationEngine::stepCompleted, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)(int , int )>(_a, &AnimationEngine::stepIndexChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)(int )>(_a, &AnimationEngine::queueSizeChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)()>(_a, &AnimationEngine::queueEmpty, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)()>(_a, &AnimationEngine::playbackStarted, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnimationEngine::*)()>(_a, &AnimationEngine::playbackStopped, 7))
            return;
    }
}

const QMetaObject *AnimationEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AnimationEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AnimationEngineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AnimationEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void AnimationEngine::frameUpdate(double _t1, const AggregatedStep & _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}

// SIGNAL 1
void AnimationEngine::stepStarted(const AggregatedStep & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void AnimationEngine::stepCompleted(const AggregatedStep & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AnimationEngine::stepIndexChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void AnimationEngine::queueSizeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void AnimationEngine::queueEmpty()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void AnimationEngine::playbackStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void AnimationEngine::playbackStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
namespace {
struct qt_meta_tag_ZN9BlockItemE_t {};
} // unnamed namespace

template <> constexpr inline auto BlockItem::qt_create_metaobjectdata<qt_meta_tag_ZN9BlockItemE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BlockItem",
        "blockOpacity",
        "blockScale",
        "glowIntensity"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'blockOpacity'
        QtMocHelpers::PropertyData<qreal>(1, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'blockScale'
        QtMocHelpers::PropertyData<qreal>(2, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'glowIntensity'
        QtMocHelpers::PropertyData<qreal>(3, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BlockItem, qt_meta_tag_ZN9BlockItemE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BlockItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BlockItemE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BlockItemE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9BlockItemE_t>.metaTypes,
    nullptr
} };

void BlockItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BlockItem *>(_o);
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<qreal*>(_v) = _t->blockOpacity(); break;
        case 1: *reinterpret_cast<qreal*>(_v) = _t->blockScale(); break;
        case 2: *reinterpret_cast<qreal*>(_v) = _t->glowIntensity(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setBlockOpacity(*reinterpret_cast<qreal*>(_v)); break;
        case 1: _t->setBlockScale(*reinterpret_cast<qreal*>(_v)); break;
        case 2: _t->setGlowIntensity(*reinterpret_cast<qreal*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *BlockItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BlockItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9BlockItemE_t>.strings))
        return static_cast<void*>(this);
    return QGraphicsObject::qt_metacast(_clname);
}

int BlockItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN18ExternalSortCanvasE_t {};
} // unnamed namespace

template <> constexpr inline auto ExternalSortCanvas::qt_create_metaobjectdata<qt_meta_tag_ZN18ExternalSortCanvasE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ExternalSortCanvas",
        "visualizationReady",
        "",
        "regionClicked",
        "region",
        "zoomChanged",
        "z",
        "onFrameUpdate",
        "dt",
        "AggregatedStep",
        "step",
        "progress",
        "onStepStarted",
        "onStepCompleted",
        "overlayOpacity"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'visualizationReady'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'regionClicked'
        QtMocHelpers::SignalData<void(const QString &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Signal 'zoomChanged'
        QtMocHelpers::SignalData<void(qreal)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 6 },
        }}),
        // Slot 'onFrameUpdate'
        QtMocHelpers::SlotData<void(double, const AggregatedStep &, double)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 8 }, { 0x80000000 | 9, 10 }, { QMetaType::Double, 11 },
        }}),
        // Slot 'onStepStarted'
        QtMocHelpers::SlotData<void(const AggregatedStep &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Slot 'onStepCompleted'
        QtMocHelpers::SlotData<void(const AggregatedStep &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'overlayOpacity'
        QtMocHelpers::PropertyData<qreal>(14, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ExternalSortCanvas, qt_meta_tag_ZN18ExternalSortCanvasE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ExternalSortCanvas::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsView::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ExternalSortCanvasE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ExternalSortCanvasE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18ExternalSortCanvasE_t>.metaTypes,
    nullptr
} };

void ExternalSortCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ExternalSortCanvas *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->visualizationReady(); break;
        case 1: _t->regionClicked((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->zoomChanged((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1]))); break;
        case 3: _t->onFrameUpdate((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<AggregatedStep>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 4: _t->onStepStarted((*reinterpret_cast<std::add_pointer_t<AggregatedStep>>(_a[1]))); break;
        case 5: _t->onStepCompleted((*reinterpret_cast<std::add_pointer_t<AggregatedStep>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ExternalSortCanvas::*)()>(_a, &ExternalSortCanvas::visualizationReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ExternalSortCanvas::*)(const QString & )>(_a, &ExternalSortCanvas::regionClicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ExternalSortCanvas::*)(qreal )>(_a, &ExternalSortCanvas::zoomChanged, 2))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<qreal*>(_v) = _t->overlayOpacity(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setOverlayOpacity(*reinterpret_cast<qreal*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *ExternalSortCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ExternalSortCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ExternalSortCanvasE_t>.strings))
        return static_cast<void*>(this);
    return QGraphicsView::qt_metacast(_clname);
}

int ExternalSortCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void ExternalSortCanvas::visualizationReady()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ExternalSortCanvas::regionClicked(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ExternalSortCanvas::zoomChanged(qreal _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP

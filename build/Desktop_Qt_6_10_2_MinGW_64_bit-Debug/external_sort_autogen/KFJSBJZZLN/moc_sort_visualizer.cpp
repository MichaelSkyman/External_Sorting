/****************************************************************************
** Meta object code from reading C++ file 'sort_visualizer.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../source/gui/visualization/sort_visualizer.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sort_visualizer.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14SortVisualizerE_t {};
} // unnamed namespace

template <> constexpr inline auto SortVisualizer::qt_create_metaobjectdata<qt_meta_tag_ZN14SortVisualizerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SortVisualizer",
        "visualizationReady",
        "",
        "regionClicked",
        "regionName",
        "zoomChanged",
        "zoomFactor",
        "viewportChanged",
        "offset",
        "visibleWidth",
        "totalWidth",
        "onStepStarted",
        "AnimationStep",
        "step",
        "onStepProgress",
        "progress",
        "onStepCompleted",
        "onFrameUpdate",
        "deltaTime",
        "stepProgress",
        "onPlaybackStarted",
        "onPlaybackStopped",
        "cameraOffsetY",
        "ramScale",
        "introProgress",
        "viewportOffset",
        "highlightIntensity",
        "transitionProgress"
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
        // Signal 'viewportChanged'
        QtMocHelpers::SignalData<void(qreal, qreal, qreal)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 8 }, { QMetaType::QReal, 9 }, { QMetaType::QReal, 10 },
        }}),
        // Slot 'onStepStarted'
        QtMocHelpers::SlotData<void(const AnimationStep &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onStepProgress'
        QtMocHelpers::SlotData<void(const AnimationStep &, double)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 }, { QMetaType::Double, 15 },
        }}),
        // Slot 'onStepCompleted'
        QtMocHelpers::SlotData<void(const AnimationStep &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onFrameUpdate'
        QtMocHelpers::SlotData<void(double, double)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 18 }, { QMetaType::Double, 19 },
        }}),
        // Slot 'onPlaybackStarted'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onPlaybackStopped'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'cameraOffsetY'
        QtMocHelpers::PropertyData<qreal>(22, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'ramScale'
        QtMocHelpers::PropertyData<qreal>(23, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'introProgress'
        QtMocHelpers::PropertyData<qreal>(24, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'zoomFactor'
        QtMocHelpers::PropertyData<qreal>(6, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'viewportOffset'
        QtMocHelpers::PropertyData<qreal>(25, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'highlightIntensity'
        QtMocHelpers::PropertyData<qreal>(26, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'transitionProgress'
        QtMocHelpers::PropertyData<qreal>(27, QMetaType::QReal, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SortVisualizer, qt_meta_tag_ZN14SortVisualizerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SortVisualizer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14SortVisualizerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14SortVisualizerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14SortVisualizerE_t>.metaTypes,
    nullptr
} };

void SortVisualizer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SortVisualizer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->visualizationReady(); break;
        case 1: _t->regionClicked((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->zoomChanged((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1]))); break;
        case 3: _t->viewportChanged((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[3]))); break;
        case 4: _t->onStepStarted((*reinterpret_cast<std::add_pointer_t<AnimationStep>>(_a[1]))); break;
        case 5: _t->onStepProgress((*reinterpret_cast<std::add_pointer_t<AnimationStep>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 6: _t->onStepCompleted((*reinterpret_cast<std::add_pointer_t<AnimationStep>>(_a[1]))); break;
        case 7: _t->onFrameUpdate((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 8: _t->onPlaybackStarted(); break;
        case 9: _t->onPlaybackStopped(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SortVisualizer::*)()>(_a, &SortVisualizer::visualizationReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SortVisualizer::*)(const QString & )>(_a, &SortVisualizer::regionClicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SortVisualizer::*)(qreal )>(_a, &SortVisualizer::zoomChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SortVisualizer::*)(qreal , qreal , qreal )>(_a, &SortVisualizer::viewportChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<qreal*>(_v) = _t->cameraOffsetY(); break;
        case 1: *reinterpret_cast<qreal*>(_v) = _t->ramScale(); break;
        case 2: *reinterpret_cast<qreal*>(_v) = _t->introProgress(); break;
        case 3: *reinterpret_cast<qreal*>(_v) = _t->zoomFactor(); break;
        case 4: *reinterpret_cast<qreal*>(_v) = _t->viewportOffset(); break;
        case 5: *reinterpret_cast<qreal*>(_v) = _t->highlightIntensity(); break;
        case 6: *reinterpret_cast<qreal*>(_v) = _t->transitionProgress(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setCameraOffsetY(*reinterpret_cast<qreal*>(_v)); break;
        case 1: _t->setRamScale(*reinterpret_cast<qreal*>(_v)); break;
        case 2: _t->setIntroProgress(*reinterpret_cast<qreal*>(_v)); break;
        case 3: _t->setZoomFactor(*reinterpret_cast<qreal*>(_v)); break;
        case 4: _t->setViewportOffset(*reinterpret_cast<qreal*>(_v)); break;
        case 5: _t->setHighlightIntensity(*reinterpret_cast<qreal*>(_v)); break;
        case 6: _t->setTransitionProgress(*reinterpret_cast<qreal*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SortVisualizer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SortVisualizer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14SortVisualizerE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SortVisualizer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void SortVisualizer::visualizationReady()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SortVisualizer::regionClicked(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void SortVisualizer::zoomChanged(qreal _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void SortVisualizer::viewportChanged(qreal _t1, qreal _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP

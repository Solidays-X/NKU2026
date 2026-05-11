/****************************************************************************
** Meta object code from reading C++ file 'gamestate.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../gamestate.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gamestate.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
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
struct qt_meta_tag_ZN9GameStateE_t {};
} // unnamed namespace

template <> constexpr inline auto GameState::qt_create_metaobjectdata<qt_meta_tag_ZN9GameStateE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GameState",
        "energyChanged",
        "",
        "sanityChanged",
        "knowledgeChanged",
        "physicalChanged",
        "loopCountChanged",
        "sinkingChanged",
        "homeworkCountChanged",
        "dayChanged",
        "DayOfWeek",
        "slotChanged",
        "TimeSlot",
        "inventoryChanged",
        "endingUnlocked",
        "Ending"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'energyChanged'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'sanityChanged'
        QtMocHelpers::SignalData<void(int)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'knowledgeChanged'
        QtMocHelpers::SignalData<void(int)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'physicalChanged'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'loopCountChanged'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'sinkingChanged'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'homeworkCountChanged'
        QtMocHelpers::SignalData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'dayChanged'
        QtMocHelpers::SignalData<void(DayOfWeek)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 2 },
        }}),
        // Signal 'slotChanged'
        QtMocHelpers::SignalData<void(TimeSlot)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 2 },
        }}),
        // Signal 'inventoryChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'endingUnlocked'
        QtMocHelpers::SignalData<void(Ending)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 15, 2 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<GameState, qt_meta_tag_ZN9GameStateE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GameState::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9GameStateE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9GameStateE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9GameStateE_t>.metaTypes,
    nullptr
} };

void GameState::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GameState *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->energyChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->sanityChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->knowledgeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->physicalChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->loopCountChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->sinkingChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->homeworkCountChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->dayChanged((*reinterpret_cast<std::add_pointer_t<DayOfWeek>>(_a[1]))); break;
        case 8: _t->slotChanged((*reinterpret_cast<std::add_pointer_t<TimeSlot>>(_a[1]))); break;
        case 9: _t->inventoryChanged(); break;
        case 10: _t->endingUnlocked((*reinterpret_cast<std::add_pointer_t<Ending>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(int )>(_a, &GameState::energyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(int )>(_a, &GameState::sanityChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(int )>(_a, &GameState::knowledgeChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(int )>(_a, &GameState::physicalChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(int )>(_a, &GameState::loopCountChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(int )>(_a, &GameState::sinkingChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(int )>(_a, &GameState::homeworkCountChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(DayOfWeek )>(_a, &GameState::dayChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(TimeSlot )>(_a, &GameState::slotChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)()>(_a, &GameState::inventoryChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameState::*)(Ending )>(_a, &GameState::endingUnlocked, 10))
            return;
    }
}

const QMetaObject *GameState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameState::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9GameStateE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GameState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void GameState::energyChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void GameState::sanityChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void GameState::knowledgeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void GameState::physicalChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void GameState::loopCountChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void GameState::sinkingChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void GameState::homeworkCountChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void GameState::dayChanged(DayOfWeek _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void GameState::slotChanged(TimeSlot _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void GameState::inventoryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void GameState::endingUnlocked(Ending _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}
QT_WARNING_POP

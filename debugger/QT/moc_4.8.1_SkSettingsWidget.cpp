/****************************************************************************
** Meta object code from reading C++ file 'SkSettingsWidget.h'
**
** Created: Mon Sep 10 14:35:18 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SkSettingsWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SkSettingsWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SkSettingsWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      35,   18,   17,   17, 0x05,
      78,   62,   17,   17, 0x05,
     104,   94,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
     138,  127,   17,   17, 0x08,
     164,  157,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SkSettingsWidget[] = {
    "SkSettingsWidget\0\0isStickyActivate\0"
    "scrollingPreferences(bool)\0isSingleCommand\0"
    "showStyle(bool)\0isEnabled\0"
    "visibilityFilter(bool)\0newCommand\0"
    "updateCommand(int)\0newHit\0updateHit(int)\0"
};

void SkSettingsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SkSettingsWidget *_t = static_cast<SkSettingsWidget *>(_o);
        switch (_id) {
        case 0: _t->scrollingPreferences((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->showStyle((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->visibilityFilter((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->updateCommand((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->updateHit((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SkSettingsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall
};

const QMetaObject SkSettingsWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SkSettingsWidget,
      qt_meta_data_SkSettingsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SkSettingsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SkSettingsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SkSettingsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SkSettingsWidget))
        return static_cast<void*>(const_cast< SkSettingsWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int SkSettingsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void SkSettingsWidget::scrollingPreferences(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SkSettingsWidget::showStyle(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SkSettingsWidget::visibilityFilter(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'paolProcess.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "paolProcess.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'paolProcess.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_paolProcess_t {
    QByteArrayData data[11];
    char stringdata[118];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_paolProcess_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_paolProcess_t qt_meta_stringdata_paolProcess = {
    {
QT_MOC_LITERAL(0, 0, 11),
QT_MOC_LITERAL(1, 12, 13),
QT_MOC_LITERAL(2, 26, 0),
QT_MOC_LITERAL(3, 27, 3),
QT_MOC_LITERAL(4, 31, 5),
QT_MOC_LITERAL(5, 37, 12),
QT_MOC_LITERAL(6, 50, 10),
QT_MOC_LITERAL(7, 61, 10),
QT_MOC_LITERAL(8, 72, 16),
QT_MOC_LITERAL(9, 89, 12),
QT_MOC_LITERAL(10, 102, 15)
    },
    "paolProcess\0capturedImage\0\0Mat\0image\0"
    "paolProcess*\0threadAddr\0savedImage\0"
    "onQuitProcessing\0onImageSaved\0"
    "onImageCaptured"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_paolProcess[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x06 /* Public */,
       7,    2,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   49,    2, 0x08 /* Private */,
       9,    1,   50,    2, 0x08 /* Private */,
      10,    1,   53,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void paolProcess::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        paolProcess *_t = static_cast<paolProcess *>(_o);
        switch (_id) {
        case 0: _t->capturedImage((*reinterpret_cast< Mat(*)>(_a[1])),(*reinterpret_cast< paolProcess*(*)>(_a[2]))); break;
        case 1: _t->savedImage((*reinterpret_cast< Mat(*)>(_a[1])),(*reinterpret_cast< paolProcess*(*)>(_a[2]))); break;
        case 2: _t->onQuitProcessing(); break;
        case 3: _t->onImageSaved((*reinterpret_cast< const Mat(*)>(_a[1]))); break;
        case 4: _t->onImageCaptured((*reinterpret_cast< const Mat(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< paolProcess* >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< paolProcess* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (paolProcess::*_t)(Mat , paolProcess * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&paolProcess::capturedImage)) {
                *result = 0;
            }
        }
        {
            typedef void (paolProcess::*_t)(Mat , paolProcess * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&paolProcess::savedImage)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject paolProcess::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_paolProcess.data,
      qt_meta_data_paolProcess,  qt_static_metacall, 0, 0}
};


const QMetaObject *paolProcess::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *paolProcess::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_paolProcess.stringdata))
        return static_cast<void*>(const_cast< paolProcess*>(this));
    return QThread::qt_metacast(_clname);
}

int paolProcess::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void paolProcess::capturedImage(Mat _t1, paolProcess * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void paolProcess::savedImage(Mat _t1, paolProcess * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE

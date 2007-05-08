#include <Python.h>
#include "structmember.h"

#include <mtag.h>

static PyObject *MTagError;

typedef struct {
    PyObject_HEAD;
	PyObject *tag;
	MTag_File *c_file;
} File;

typedef struct {
    PyObject_HEAD;
	PyObject *file;
	MTag_Tag *c_tag;
} Tag;

static PyTypeObject FileType =
{
    PyObject_HEAD_INIT (NULL)
};

static PyTypeObject TagType =
{
    PyObject_HEAD_INIT (NULL)
};

static void
Tag_dealloc (Tag* self)
{
	self->ob_type->tp_free ((PyObject*)self);
}

static int
Tag_init (PyObject *self,
		  PyObject *args,
		  PyObject *kwords)
{
	Tag *c_self;
	PyObject *file;

	c_self = (Tag *) self;

	if (!PyArg_ParseTuple (args, "O", &file))
	{
		return -1;
	}

	c_self->file = file;

	c_self->c_tag = mtag_file_tag (((File *)file)->c_file);

	return 0;
}

static PyObject *
Tag_get (Tag *self,
		 PyObject *args)
{
	char *key;

	PyArg_ParseTuple (args, "s", &key);

	return Py_BuildValue ("s", mtag_tag_get (self->c_tag, key));
}

static PyObject *
Tag_set (Tag *self,
		 PyObject *args)
{
	char *key;
	char *value;

	PyArg_ParseTuple (args, "ss", &key, &value);

	mtag_tag_set (self->c_tag, key, value);

	Py_INCREF (Py_None);
    return Py_None;
}

static void
File_dealloc (File* self)
{
	mtag_file_free (self->c_file);

	self->ob_type->tp_free ((PyObject*)self);
}

static int
File_init (PyObject *self,
		   PyObject *args,
		   PyObject *kwords)
{
	File *c_self;
	const char *c_file_name;

	c_self = (File *) self;

	if (!PyArg_ParseTuple (args, "s", &c_file_name))
	{
		return -1;
	}

	c_self->c_file = mtag_file_new (c_file_name);

	if (!c_self->c_file)
	{
		/** @todo raise exception. */
	}

	return 0;
}

static PyObject *
File_tag (File *self,
		  PyObject *args)
{
	if (!self->tag)
	{
		self->tag = (PyObject *) PyObject_New (Tag, &TagType);
		PyObject_CallMethod (self->tag, "__init__", "O", self);
	}

	Py_INCREF (self->tag);
    return self->tag;
}

static PyObject *
File_save (File *self,
		   PyObject *args)
{
	mtag_file_save (self->c_file);

	Py_INCREF (Py_None);
    return Py_None;
}

static PyMethodDef File_methods[] =
{
	{"tag", (PyCFunction)File_tag, METH_NOARGS, "Tag."},
	{"save", (PyCFunction)File_save, METH_NOARGS, "Save."},
	{NULL}
};

static PyMethodDef Tag_methods[] =
{
	{"get", (PyCFunction)Tag_get, METH_VARARGS, "Get."},
	{"set", (PyCFunction)Tag_set, METH_VARARGS, "Set."},
	{NULL}
};

static PyMethodDef MTagMethods[] =
{
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initlibmtag (void)
{
	PyObject *m;

	m = Py_InitModule ("libmtag", MTagMethods);

	TagType.tp_new = PyType_GenericNew;
	TagType.tp_name = "Tag";
	TagType.tp_basicsize = sizeof (Tag);
	TagType.tp_dealloc = (destructor) Tag_dealloc;
	TagType.tp_flags = Py_TPFLAGS_DEFAULT;
	TagType.tp_methods = Tag_methods;
	TagType.tp_init = Tag_init;

	FileType.tp_new = PyType_GenericNew;
	FileType.tp_name = "File";
	FileType.tp_basicsize = sizeof (File);
	FileType.tp_dealloc = (destructor) File_dealloc;
	FileType.tp_flags = Py_TPFLAGS_DEFAULT;
	FileType.tp_methods = File_methods;
	FileType.tp_init = File_init;

	if (PyType_Ready (&FileType) < 0)
		return;

	if (PyType_Ready (&TagType) < 0)
		return;

	PyModule_AddObject (m, "Tag", (PyObject *)&TagType);
	PyModule_AddObject (m, "File", (PyObject *)&FileType);

	MTagError = PyErr_NewException ("libmtag.error", NULL, NULL);
	Py_INCREF (MTagError);
	PyModule_AddObject (m, "error", MTagError);
}

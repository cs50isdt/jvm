#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "class.h"
#include "memory.h"
#include "native.h"

static void
natprintln(Frame *frame, char *type)
{
	Value vfp, v;

	v = frame_stackpop(frame);
	if (strcmp(type, "()V") == 0) {
		fprintf((FILE *)v.v->obj, "\n");
	} else {
		vfp = frame_stackpop(frame);
		if (strcmp(type, "(Ljava/lang/String;)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%s\n", (char *)v.v->obj);
		else if (strcmp(type, "(B)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d\n", v.i);
		else if (strcmp(type, "(C)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%c\n", v.i);
		else if (strcmp(type, "(D)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%.16g\n", v.d);
		else if (strcmp(type, "(F)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%.16g\n", v.f);
		else if (strcmp(type, "(I)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d\n", v.i);
		else if (strcmp(type, "(J)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%lld\n", (long long int)v.l);
		else if (strcmp(type, "(S)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d\n", v.i);
		else if (strcmp(type, "(Z)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d\n", v.i);
	}
}

static void
natprint(Frame *frame, char *type)
{
	Value vfp, v;

	v = frame_stackpop(frame);
	if (strcmp(type, "()V") != 0) {
		vfp = frame_stackpop(frame);
		if (strcmp(type, "(Ljava/lang/String;)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%s", (char *)v.v->obj);
		else if (strcmp(type, "(B)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d", v.i);
		else if (strcmp(type, "(C)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%c", v.i);
		else if (strcmp(type, "(D)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%.16g", v.d);
		else if (strcmp(type, "(F)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%.16g", v.f);
		else if (strcmp(type, "(I)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d", v.i);
		else if (strcmp(type, "(J)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%lld", (long long int)v.l);
		else if (strcmp(type, "(S)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d", v.i);
		else if (strcmp(type, "(Z)V") == 0)
			fprintf((FILE *)vfp.v->obj, "%d", v.i);
	}
}

static void
natstringcharat(Frame *frame, char *type)
{
	Value index, receiver, result;
	const char *str;

	// TODO(max): UTF-16 index
	assert(strcmp(type, "(I)C") == 0);
	index = frame_stackpop(frame);
	receiver = frame_stackpop(frame);
	str = (char *)receiver.v->obj;
	result.i = str[index.i];
	frame_stackpush(frame, result);
}

static void
natstringlength(Frame *frame, char *type)
{
	Value receiver, result;
	const char *str;

	// TODO(max): UTF-16
	assert(strcmp(type, "()I") == 0);
	receiver = frame_stackpop(frame);
	str = (char *)receiver.v->obj;
	result.i = strlen(str);
	frame_stackpush(frame, result);
}

static struct {
	char *name;
	JavaClass jclass;
} jclasstab[] = {
	{"java/lang/System",    LANG_SYSTEM},
	{"java/lang/String",    LANG_STRING},
	{"java/io/PrintStream", IO_PRINTSTREAM},
	{NULL,                  NONE_CLASS},
};

static struct Native {
	const char *name;
	void (*method)(Frame *frame, char *type);
} *nativetab[] = {
	[IO_PRINTSTREAM] = (struct Native[]){
		{"print", natprint},
		{"println", natprintln},
		{NULL, NULL},
	},
	[LANG_STRING] = (struct Native[]){
		{"charAt", natstringcharat},
		{"length", natstringlength},
		{NULL, NULL},
	},
	[LANG_SYSTEM] = (struct Native[]){
		{NULL, NULL},
	},
};

JavaClass
native_javaclass(char *classname)
{
	size_t i;

	for (i = 0; jclasstab[i].name; i++)
		if (strcmp(classname, jclasstab[i].name) == 0)
			break;
	return jclasstab[i].jclass;
}

void *
native_javaobj(JavaClass jclass, char *objname, char *objtype)
{
	switch (jclass) {
	default:
		break;
	case LANG_SYSTEM:
		if (strcmp(objtype, "Ljava/io/PrintStream;") == 0) {
			if (strcmp(objname, "out") == 0) {
				return stdout;
			} else if (strcmp(objname, "err") == 0) {
				return stderr;
			} else if (strcmp(objname, "in") == 0) {
				return stdin;
			}
		}
		break;
	}
	return NULL;
}

int
native_javamethod(Frame *frame, JavaClass jclass, char *name, char *type)
{
	U8 i;

	for (i = 0; nativetab[jclass][i].name != NULL; i++) {
		if (strcmp(name, nativetab[jclass][i].name) == 0) {
			nativetab[jclass][i].method(frame, type);
			return 0;
		}
	}
	return -1;
}

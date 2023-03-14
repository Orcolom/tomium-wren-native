#include <stdio.h>
#include <string.h>

#include "foreign_class.h"

static int finalized = 0;

// WrenForeignClassMethods.userData testing:
// - Counter and Point have different userData values.
// - We test allocate() by retrieving the userData value from the
//   created instance.
// - We test finalize() by storing the userData value into this file and
//   retrieving it using a static property of Point.
static const double counterUserData = 12345;
static const double pointUserData = 100200;

// Where pointFinalize will store userData
static double pointFinalizeResult = 404;

// Instance of a Point or Counter
typedef struct Instance
{
  double instanceUserData;
  double values[3];   //Counter only uses [0]
} Instance;

static void apiFinalized(WrenVM* vm, void *userData)
{
  wrenSetSlotDouble(vm, 0, finalized);
}

static void counterAllocate(WrenVM* vm, void *userData)
{
  Instance* instance = (Instance*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Instance));
  instance->values[0] = 0;
  instance->instanceUserData = *(double *)userData;
}

// Return a foreign class's userdata
static void getClassUserData(WrenVM* vm, void *userData)
{
  Instance* instance = (Instance*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, instance->instanceUserData);
}

static void counterIncrement(WrenVM* vm, void *userData)
{
  Instance* instance = (Instance*)wrenGetSlotForeign(vm, 0);
  double increment = wrenGetSlotDouble(vm, 1);

  instance->values[0] += increment;
}

static void counterValue(WrenVM* vm, void *userData)
{
  Instance* instance = (Instance*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, instance->values[0]);
}

static void pointAllocate(WrenVM* vm, void *userData)
{
  Instance* instance = (Instance*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Instance));
  instance->instanceUserData = *(double *)userData;

  // This gets called by both constructors, so sniff the slot count to see
  // which one was invoked.
  if (wrenGetSlotCount(vm) == 1)
  {
    instance->values[0] = 0.0;
    instance->values[1] = 0.0;
    instance->values[2] = 0.0;
  }
  else
  {
    instance->values[0] = wrenGetSlotDouble(vm, 1);
    instance->values[1] = wrenGetSlotDouble(vm, 2);
    instance->values[2] = wrenGetSlotDouble(vm, 3);
  }
}

static void pointFinalize(void *data, void *userData)
{
  pointFinalizeResult = *(double *)userData;
}

static void pointGetFinalizeResult(WrenVM* vm, void *userData)
{
  wrenSetSlotDouble(vm, 0, pointFinalizeResult);
}

static void pointTranslate(WrenVM* vm, void *userData)
{
  Instance* instance = (Instance*)wrenGetSlotForeign(vm, 0);
  instance->values[0] += wrenGetSlotDouble(vm, 1);
  instance->values[1] += wrenGetSlotDouble(vm, 2);
  instance->values[2] += wrenGetSlotDouble(vm, 3);
}

static void pointToString(WrenVM* vm, void *userData)
{
  Instance* instance = (Instance*)wrenGetSlotForeign(vm, 0);
  char result[100];
  sprintf(result, "(%g, %g, %g)",
      instance->values[0], instance->values[1], instance->values[2]);
  wrenSetSlotString(vm, 0, result);
}

static void resourceAllocate(WrenVM* vm, void *userData)
{
  int* value = (int*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(int));
  *value = 123;
}

static void resourceFinalize(void* data, void *userData)
{
  // Make sure we get the right data back.
  int* value = (int*)data;
  if (*value != 123) exit(1);

  finalized++;
}

static void badClassAllocate(WrenVM* vm, void *userData)
{
  wrenEnsureSlots(vm, 1);
  wrenSetSlotString(vm, 0, "Something went wrong");
  wrenAbortFiber(vm, 0);
}

WrenForeignMethodFn foreignClassBindMethod(const char* signature)
{
  if (strcmp(signature, "static ForeignClass.finalized") == 0) return apiFinalized;
  if (strcmp(signature, "Counter.increment(_)") == 0) return counterIncrement;
  if (strcmp(signature, "Counter.value") == 0) return counterValue;
  if (strcmp(signature, "Point.translate(_,_,_)") == 0) return pointTranslate;
  if (strcmp(signature, "Point.toString") == 0) return pointToString;
  if (strcmp(signature, "Counter.instanceUserData") == 0) return getClassUserData;
  if (strcmp(signature, "Point.instanceUserData") == 0) return getClassUserData;
  if (strcmp(signature, "static Point.finalizeResult") == 0) return pointGetFinalizeResult;

  return NULL;
}

void foreignClassBindClass(
    const char* className, WrenForeignClassMethods* methods)
{
  if (strcmp(className, "Counter") == 0)
  {
    methods->allocate = counterAllocate;
    methods->userData = (void *)&counterUserData;
    return;
  }

  if (strcmp(className, "Point") == 0)
  {
    methods->allocate = pointAllocate;
    methods->finalize = pointFinalize;
    methods->userData = (void *)&pointUserData;
    return;
  }

  if (strcmp(className, "Resource") == 0)
  {
    methods->allocate = resourceAllocate;
    methods->finalize = resourceFinalize;
    return;
  }

  if (strcmp(className, "BadClass") == 0)
  {
    methods->allocate = badClassAllocate;
    return;
  }
}

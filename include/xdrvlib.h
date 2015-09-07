#ifndef XDRVLIB_H
#define XDRVLIB_H
/*|--------------------------------------------------------------------------

 MAGELLAN X-Window Driver Header                        Version 3.01 05.10.95

 Logitech Inc.
 6505 Kaiser Drive
 Fremont, CA 94555-3615

 Volker Senft:

 Main:   (510) 795-8500
 Direct: (510) 713-4055
 Sales:  (800) 231-7717
 Fax:    (510) 505-0979

----------------------------------------------------------------------------|*/

#ifndef TRUE
#define TRUE	(1==1)
#endif

#ifndef FALSE
#define FALSE	(1==0)
#endif

#ifdef SGI
#define ParameterCheck
#endif

#ifdef IBM
#define ParameterCheck
#endif

/* added by AB */

#ifdef SV_UNIX
#define ParameterCheck
#endif

/* Data Structure for CADDS5 implementation */
#define MagellanInputMotionEvent 1
#define MagellanInputButtonPressEvent 2
#define MagellanInputButtonReleaseEvent 3

union _MagellanInputEventUnion_ {
 int data[7];
 int button;
};
typedef union _MagellanInputEventUnion_ MagellanIntUnion;

struct _MagellanIntEvent_ {
 int type;
 MagellanIntUnion u;
};
typedef struct _MagellanIntEvent_ MagellanIntEvent;


/* Data Structure for Pro/ENGINEER implementation */
enum _MagellanData_ { MagellanX,MagellanY,MagellanZ,MagellanA,MagellanB,MagellanC };

struct _MagellanFloatEvent_ {
 int MagellanType;
 int MagellanButton;
 double MagellanData[6];
 int MagellanPeriod;
};
typedef struct _MagellanFloatEvent_ MagellanFloatEvent;



int MagellanInit(
#ifdef ParameterCheck
Display *display,
Window window
#endif
);

int MagellanSetWindow(
#ifdef ParameterCheck
Display *display,
Window window
#endif
);

int MagellanApplicationSensitivity(
#ifdef ParameterCheck
Display *display,
double Sensitivity
#endif
);

static int MagellanErrorHandler(
#ifdef ParameterCheck
Display *display,
XErrorEvent *Error
#endif
);

int MagellanInputEvent(
#ifdef ParameterCheck
Display *display,
XEvent *Event,
MagellanIntEvent *MagellanEvent
#endif
);

int MagellanTranslateEvent(
#ifdef ParameterCheck
Display *display,
XEvent *Event,
MagellanFloatEvent *MagellanEvent,
double MagellanTranslationScale,
double MagellanRotationScale
#endif
);

int MagellanClose(
#ifdef ParameterCheck
Display *display
#endif
);

int MagellanRotationMatrix(
#ifdef ParameterCheck
double Rotate[4][4],
double c,
double b,
double a
#endif
);

int MagellanMultiplicationMatrix(
#ifdef ParameterCheck
double MatrixA[4][4],
double MatrixB[4][4],
double MatrixC[4][4] 
#endif 
);

extern Atom MagellanMotionEvent, MagellanButtonPressEvent, MagellanReleaseEvent;
extern Window MagellanWindow;
extern int MagellanExist;

enum _CommandMessages_ { NoCommandMessage, CommandMessageApplicationWindow = 27695, 
                                           CommandMessageApplicationSensitivity };
union _MagellanTypeConversion_ {
 float Float;
 short Short[ 2 ];
};
typedef union _MagellanTypeConversion_ MagellanTypeConversion;

#define XHigh32( Value )        (((Value)>>16)&0x0000FFFF)
#define XLow32( Value )         ((Value)&0x0000FFFF)

#endif  /* Magellan X Window Driver Header */


#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include <math.h>


#define kTableLength 4096


// VectorOscFM~


// object data structure
typedef struct {
    t_pxobject x_obj;
    double increment;
    double incrementM;
    double freq;
    double modfreq;
    double modp;
    double index;
    double indexM;
    double sr;
    double *waveTable;
    double *waveTable2;
    double *waveTable3;
    double *waveTable4;
    double *waveTableM;
    double incrementF;
    double indexAmp;
    double ampx;
    double ampy;
} sine;

t_class *myClass;			// global variable to contain class

t_symbol *sineMessC;
t_symbol *squareMessC;
t_symbol *sawMessC;

t_symbol *sineMessM;
t_symbol *squareMessM;
t_symbol *sawMessM;
t_symbol *triMessM;



///methods declaration
void *SineNew(double freq, double modfreq, double modp) ;
void SineFree(sine *x);
void SineFloat(sine *x, double freq);
void SineFloat1(sine *x, double modfreq);
void SineFloat2(sine *x, double modp);
void SineFloat3(sine *x, double ampx);
void SineFloat4(sine *x, double ampy);
void SineInt(sine *x, long n);
void SineInt1(sine *x, long modn);
void SineInt2(sine *x, long modpn);
void SineInt3(sine *x, long ax);
void SineInt4(sine *x, long ay);
void SineDsp64(sine *x, t_object *dsp64,short *count, double samplerate, long maxvectorsize, long flags);
void SineAssist(sine *x, void *b, long message, long argument, char *s);
void SinePerform64(sine *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void SineTable(sine *x);
void TriTable(sine *x);
void SquTable(sine *x);
void SawTable(sine *x);
void SineTableM(sine *x);
void SquTableM(sine *x);
void SawTableM(sine *x);
void TriTableM(sine *x);
void SineSet(sine *x, t_symbol *s);
void SineBang(sine *x);
void harmTable(sine *x);



/////MAIN
int C74_EXPORT main(void) {
	t_class *c;
	c = class_new("VectorOsc~", (method) SineNew, (method) dsp_free, sizeof(sine), 0, A_GIMME, 0);
    
    // bind a dsp method
	class_addmethod(c, (method) SineDsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method) SineFloat, "float", A_FLOAT, 0);
    class_addmethod(c, (method) SineFloat1, "ft1", A_FLOAT, 0);
    class_addmethod(c, (method) SineFloat2, "ft2", A_FLOAT, 0);
    class_addmethod(c, (method) SineFloat3, "ft3", A_FLOAT, 0);
        class_addmethod(c, (method) SineFloat4, "ft4", A_FLOAT, 0);
	class_addmethod(c, (method) SineInt, "int", A_LONG, 0);
    class_addmethod(c, (method) SineInt1, "in1", A_LONG, 0);
    class_addmethod(c, (method) SineInt2, "in2", A_LONG, 0);
    class_addmethod(c, (method) SineInt3, "in3", A_LONG, 0);
        class_addmethod(c, (method) SineInt4, "in4", A_LONG, 0);
	class_addmethod(c, (method) SineAssist, "assist", A_CANT, 0);
    class_addmethod(c, (method) SineSet, "set", A_SYM, 0);
    class_addmethod(c, (method) SineBang, "bang", 0);
    
    // initialize dsp class
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	myClass = c;
    sineMessC = gensym("Csine");
    squareMessC = gensym("Csquare");
    sawMessC = gensym("Csaw");
    sineMessM = gensym("Msine");
    squareMessM = gensym("Msquare");
    sawMessM = gensym("Msaw");
    triMessM = gensym("Mtri");
    
	return 0;
}




// assist method (for help strings)
void SineAssist(sine *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_OUTLET)
		sprintf(s, "Signal Output");
	else
        switch(a) {
            case 0: sprintf(s, "Carrier Freq");
                break;
            case 1: sprintf(s, "Mod Freq");
                break;
            case 2: sprintf(s, "Mod Depth");
                break;
            case 3: sprintf(s, "Wave X");
                break;
            case 4: sprintf(s, "Wave Y");
                break;
        }
}

// method to create a new instance of the object
void *SineNew(double freq, double modfreq, double modp) {
	sine *x;
    
   	x = (sine *) object_alloc(myClass);
    dsp_setup((t_pxobject *)x,0);
   
        floatin(x,4);
    floatin(x,3);
    floatin(x,2);
    floatin(x,1);
    

    
    outlet_new((t_pxobject *)x, "signal");

    
    x->ampx = 0.5;
        x->ampy = 0.5;
    x->freq = freq;
    x->modfreq = modfreq;
    x->modp = modp;
    x->waveTable = (double *)sysmem_newptr(sizeof(double) * kTableLength);
    x->waveTableM = (double *)sysmem_newptr(sizeof(double) * kTableLength);
    x->waveTable2 = (double *)sysmem_newptr(sizeof(double) * kTableLength);
        x->waveTable3 = (double *)sysmem_newptr(sizeof(double) * kTableLength);
        x->waveTable4 = (double *)sysmem_newptr(sizeof(double) * kTableLength); 

	SawTable(x);
    SquTable(x);
    SineTable(x);
    TriTable(x);
    SquTableM(x);
    return (x);
}

void SineFree(sine *x) {
	if (x->waveTable) // if there is a pointer to a wavetable
		sysmem_freeptr(x->waveTable);
    
    if (x->waveTableM) // if there is a pointer to a wavetable
		sysmem_freeptr(x->waveTableM); //then free the storage
    
    if (x->waveTable2) // if there is a pointer to a wavetable
		sysmem_freeptr(x->waveTable2); //then free the storage
    
    if (x->waveTable3) // if there is a pointer to a wavetable
		sysmem_freeptr(x->waveTable3); //then free the storage
    
    if (x->waveTable4) // if there is a pointer to a wavetable
		sysmem_freeptr(x->waveTable4); //then free the storage
    
	dsp_free((t_pxobject *)x); //call the standard msp free routine
}

// method for float input

void SineFloat(sine *x, double freq){
    freq = fabsl(freq);
    x->freq = freq;
    x->increment = freq * kTableLength / x->sr;
}

void SineFloat1(sine *x,double modfreq) {
    modfreq = fabsl(modfreq);
    x->modfreq = modfreq;
    x->incrementM = modfreq * kTableLength / x->sr;
}

void SineFloat2(sine *x,double modp) {
    modp = fabsl(modp);
    x->modp = modp;
}

void SineFloat3(sine *x,double ampx) {
    ampx = fabsl(ampx),
    x->ampx = ampx;
    
    
}

void SineFloat4(sine *x,double ampy) {
    ampy = fabsl(ampy),
    x->ampy = ampy;
    
    
}

// method for int (long) input
void SineInt(sine *x, long n) {
    n = abs(n),
    x->freq = n,
    x->increment = (double) n * kTableLength / x->sr;
}


void SineInt1(sine *x,long modn) {
    modn = abs(modn),
    x->modfreq = modn,
    x->incrementM = modn * kTableLength / x->sr;
}


void SineInt2(sine *x,long modpn) {
    modpn = abs(modpn),
    x->modp = modpn;
    
}


void SineInt3(sine *x,long ax) {
    ax = abs(ax),
    x->ampx = ax;
    
    
}

void SineInt4(sine *x,long ay) {
    ay = abs(ay),
    x->ampy = ay;
    
    
}

///method for bang

void SineBang(sine *x) {
    x->indexAmp = 0.0;
    
}


///method for set


void SineSet(sine *x, t_symbol *s) {
    if (s == sineMessM)
        SineTableM(x);
    else if (s == squareMessM)
        SquTableM(x);
    else  if (s == sawMessM)
        SawTableM(x);
    else  if (s == triMessM)
        TriTableM(x);
};

// 64-bit dsp method

void SineDsp64(sine *x, t_object *dsp64,short *count, double samplerate, long maxvectorsize, long flags) {
	x->sr = samplerate;
    
	x->increment = x->freq * kTableLength/samplerate;
    
    x->incrementM = x->modfreq * kTableLength/samplerate;
    
    
    
	object_method(dsp64, gensym("dsp_add64"), x, SinePerform64, 0, NULL);
}



// dsp64 perform method
void SinePerform64 (sine *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
    double increment,index,*table, incrementM, modp, *tableM, indexM, mod, ampx, ampy, *table2,*table3,*table4;
    
    increment = x->increment;
    index = x->index;
    table = x->waveTable;
    table2 = x->waveTable2;
    table3 = x->waveTable3;
    table4 = x->waveTable4;
    tableM = x->waveTableM;
    incrementM = x->incrementM;
    modp = x->modp;
    indexM = x->indexM;
    ampx = x->ampx;
    ampy = x->ampy;
    
	t_double *out1 = outs[0];
    
    
   	
	while (sampleframes--) 			// fill output buffer
    {
        
        
        
        mod = modp * tableM[(int)indexM];    //modulator
        indexM += incrementM;
        while(indexM >= kTableLength)
            indexM -= kTableLength;

        
        
        *out1++ = (((ampx * table[(int)index]) + ((1.0 - ampx)*table2[(int)index]))*ampy) +  ((ampx * table3[(int)index]) + ((1.0 - ampx)*table4[(int)index]))*(1.0 - ampy) ;      //carrier

        mod = fabsl(mod);
        index += (increment+mod);
        while(index >= kTableLength)
            index -= kTableLength;
        
        
        
    }
    x->increment = increment;
    x->incrementM = incrementM;
    x->index = index;
    x->indexM = indexM;
    x->modp = modp;
    x->ampx = ampx;
    x->ampy = ampy;
    
}


/////////Waveforms

/////carrier

void SineTable(sine *x )
{
    int i;
   	for (i = 0; i < kTableLength; i++)
		x->waveTable[i] = sin(TWOPI* i / kTableLength);
}

void SquTable(sine *x)
{
    int i;
   	for (i = 0; i < (kTableLength/2); i++)
		x->waveTable3[i] = 0.99;
    for (i = (kTableLength/2); i < kTableLength; i++)
		x->waveTable3[i] = -0.99;
}

void SawTable(sine *x)
{
    int i;
   	for (i = 0.0; i < kTableLength; i++)
		x->waveTable4[i] = (((i/4096.0)- 0.5 ) *2.0 );
    
}

void TriTable(sine *x)
{
    int i;
    int c = kTableLength/2 ;
   	for (i = 0.0; i < kTableLength/2; i++)
		x->waveTable2[i] = (((i/4096.0)- 0.25 ) *4.0 );
    for (i = kTableLength/2; i < kTableLength; i++, c--)
		x->waveTable2[i] = (((c/4096.0)- 0.25 ) *4.0 );
    
}



////modulator

void SineTableM(sine *x )
{
    int i;
   	for (i = 0; i < kTableLength; i++)
		x->waveTableM[i] = sin(TWOPI* i / kTableLength);
}

void SquTableM(sine *x)
{
    int i;
   	for (i = 0; i < (kTableLength/2); i++)
		x->waveTableM[i] = 0.99;
    for (i = (kTableLength/2); i < kTableLength; i++)
		x->waveTableM[i] = -0.99;
}

void SawTableM(sine *x)
{
    int i;
   	for (i = 0.0; i < kTableLength; i++)
		x->waveTableM[i] = (((i/4096.0)- 0.5 ) *2.0 );
    
}

void TriTableM(sine *x)
{
    int i;
    int c = kTableLength/2 ;
   	for (i = 0.0; i < kTableLength/2; i++)
		x->waveTableM[i] = (((i/4096.0)- 0.25 ) *4.0 );
    for (i = kTableLength/2; i < kTableLength; i++, c--)
		x->waveTableM[i] = (((c/4096.0)- 0.25 ) *4.0 );
    
}



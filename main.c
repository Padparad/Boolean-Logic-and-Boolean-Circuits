#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stdbool.h"
#include <string.h>

char strinput[1024];

typedef struct Value {
    bool value;
} Value;

Value* new_Value(bool value) {
    Value* this = (Value*)malloc(sizeof(Value));
    this->value = value;
    return this;
}

Value** new_Value_array(int len) {
    Value** this = (Value**)calloc(sizeof(Value*), len);
    return this;
}

bool Value_getValue(Value* this) {
    return this->value;
}

void Value_setValue(Value* this, bool b) {
    this->value = b;
}

typedef struct Gate {
    int numInputs;
    Value **inputs;
    Value *output;
    void (*update)(struct Gate*);
} Gate;

static Gate* new_Gate() {
    Gate* this = (Gate*)malloc(sizeof(Gate));
    this->output = new_Value(false);
    return this;
}

Gate** new_Gate_array(int len) {
    Gate** this = (Gate**)calloc(sizeof(Gate*), len);
    return this;
}

void Gate_update(Gate *this) {
    this->update(this);
}


Value* Gate_getOutput(Gate* this) {
    return this->output;
}

bool Gate_hasInput(Gate* this, Value* value) {
    for (int i=0; i < this->numInputs; i++) {
        if (this->inputs[i] == value) {
            return true;
        }
    }
    return false;
}

bool Gate_feedsInto(Gate* this, Gate* other) {
    return Gate_hasInput(other, this->output);
}

static Gate* new_UnaryGate() {
    Gate* this = new_Gate();
    this->numInputs = 1;
    this->inputs = new_Value_array(this->numInputs);
    return this;
}


static Gate* new_BinaryGate() {
    Gate* this = new_Gate();
    this->numInputs = 2;
    this->inputs = new_Value_array(this->numInputs);
    return this;
}


static void Inverter_update(Gate *this) {
    Value_setValue(this->output, !Value_getValue(this->inputs[0]));
}


Gate* new_Inverter(Value *input) {
    Gate* this = new_UnaryGate();
    this->inputs[0] = input;
    this->update = Inverter_update;
    return this;
}

static void AndGate_update(Gate *this) {
    Value_setValue(this->output, Value_getValue(this->inputs[0]) && Value_getValue(this->inputs[1]));
}


Gate* new_AndGate(Value *input1, Value *input2) {
    Gate* this = new_BinaryGate();
    this->inputs[0] = input1;
    this->inputs[1] = input2;
    this->update = AndGate_update;
    return this;
}



static void OrGate_update(Gate *this) {
    Value_setValue(this->output, Value_getValue(this->inputs[0]) || Value_getValue(this->inputs[1]));
}


Gate* new_OrGate(Value *input1, Value *input2) {
    Gate* this = new_BinaryGate();
    this->inputs[0] = input1;
    this->inputs[1] = input2;
    this->update = OrGate_update;
    return this;
}

static void NorGate_update(Gate *this) {
    Value_setValue(this->output, (!Value_getValue(this->inputs[0])) && (!Value_getValue(this->inputs[1])));
}


Gate* new_NorGate(Value *input1, Value *input2) {
    Gate* this = new_BinaryGate();
    this->inputs[0] = input1;
    this->inputs[1] = input2;
    this->update = NorGate_update;
    return this;
}

static void And3Gate_update(Gate *this) {
    Value_setValue(this->output, Value_getValue(this->inputs[0]) && Value_getValue(this->inputs[1]) && Value_getValue(this->inputs[2]));
}


Gate* new_And3Gate(Value *input1, Value *input2, Value* input3) {
    Gate* this = new_Gate();
    this->numInputs = 3;
    this->inputs = new_Value_array(this->numInputs);
    this->inputs[0] = input1;
    this->inputs[1] = input2;
    this->inputs[2] = input3;
    this->update = And3Gate_update;
    return this;
}

static void Or4Gate_update(Gate *this)
{
    Value_setValue(this->output, Value_getValue(this->inputs[0]) || Value_getValue(this->inputs[1]) || Value_getValue(this->inputs[2]) || Value_getValue(this->inputs[3]));
}

Gate* new_Or4Gate(Value *input1, Value *input2, Value* input3, Value* input4)
{
    Gate* this = new_Gate();
    this->numInputs = 4;
    this->inputs = new_Value_array(this->numInputs);
    this->inputs[0] = input1;
    this->inputs[1] = input2;
    this->inputs[2] = input3;
    this->inputs[3] = input4;
    this->update = Or4Gate_update;
    return this;
}

typedef struct Circuit
{
    int numInputs;
    Value** inputs;
    int numOutputs;
    Value** outputs;
    int numGates;
    Gate** gates;
} Circuit;

static Gate** topsort(int ngates, Gate** gates)
{
    for (int i=1; i < ngates-1; i++) {
        int j = i;
        while (j > 0 && Gate_feedsInto(gates[j], gates[j-1])) {
            Gate *tmp = gates[j];
            gates[j] = gates[j-1];
            gates[j-1] = tmp;
            j = j - 1;
        }
    }
    return gates;
}

Circuit *new_Circuit(int numInputs, Value** inputs, int numOutputs, Value** outputs, int numGates, Gate** gates)
{
    Circuit* this = (Circuit*)malloc(sizeof(Circuit));
    this->numInputs = numInputs;
    this->inputs = inputs;
    this->numOutputs = numOutputs;
    this->outputs = outputs;
    this->numGates = numGates;
    this->gates = topsort(numGates, gates);
    return this;
}

void Circuit_update(Circuit* this)
{
    for (int i=0; i < this->numGates; i++)
    {
        Gate_update(this->gates[i]);
    }
}

int Circuit_numInputs(Circuit* this)
{
    return this->numInputs;
}


void Circuit_setInput(Circuit* this, int index, bool value)
{
    Value_setValue(this->inputs[index], value);
}

int Circuit_numOutputs(Circuit* this)
{
    return this->numOutputs;
}


bool Circuit_getOutput(Circuit* this, int index)
{
    return Value_getValue(this->outputs[index]);
}

static char* b2s(bool b)
{
    return b ? "1" : "0";
}

void test_all(Circuit* c){
    int cols = Circuit_numInputs(c);
    int rows = pow(2.0, cols);

    for(int i=0; i<rows; i++)
    {
        int binaryRowNum[cols];
        for(int j=0; j<cols; j++)
        {
            binaryRowNum[cols-j-1] = (i >> j) & 1;
        }

        for(int k=0; k<cols; k++)
        {
            Circuit_setInput(c, k, binaryRowNum[k]);
            printf("%s ", b2s(binaryRowNum[k]));
        }
        Circuit_update(c);
        printf("-> ");

        for(int l=0; l<Circuit_numOutputs(c); l++)
        {
            printf("%s ", b2s(Circuit_getOutput(c, l)));
        }
        printf("\n");
    }
}

Circuit* build_circuit_A(){
    Value* inx = new_Value(false);
    Value* iny = new_Value(false);

    Gate* not1 = new_Inverter(inx);
    Gate* and1 = new_AndGate(inx, iny);
    Gate* or1  = new_OrGate(Gate_getOutput(not1), Gate_getOutput(and1));

    Gate** gates = new_Gate_array(3);
    gates[0] = not1;
    gates[1] = and1;
    gates[2] = or1;

    Value** inputs = new_Value_array(2);
    inputs[0] = inx;
    inputs[1] = iny;

    Value** outputs = new_Value_array(1);
    outputs[0] = Gate_getOutput(or1);

    return new_Circuit(2, inputs, 1, outputs, 3, gates);
}

Circuit* build_circuit_B(){

    Value* inx = new_Value(false);
    Value* iny = new_Value(false);

    Gate* nor1 = new_NorGate(inx,iny);
    Gate* nor2 = new_NorGate(inx,iny);
    Gate* nor3 = new_NorGate(Gate_getOutput(nor1), Gate_getOutput(nor2));

    Gate** gates = new_Gate_array(3);
    gates[0] = nor1;
    gates[1] = nor2;
    gates[2] = nor3;

    Value** inputs = new_Value_array(2);
    inputs[0] = inx;
    inputs[1] = iny;

    Value** outputs = new_Value_array(1);
    outputs[0] = Gate_getOutput(nor3);

    return new_Circuit(2, inputs, 1, outputs, 3, gates);
}

Circuit* build_circuit_C(){

    Value* inx = new_Value(false);
    Value* iny = new_Value(false);
    Value* inz = new_Value(false);

    Gate* not1 = new_Inverter(iny);
    Gate* not2 = new_Inverter(inz);
    Gate* not3 = new_Inverter(inx);

    Gate* or1  = new_OrGate(inx, Gate_getOutput(not1));
    Gate* or2  = new_OrGate(iny, Gate_getOutput(not2));
    Gate* or3  = new_OrGate(inz, Gate_getOutput(not3));

    Gate* and1 = new_AndGate(Gate_getOutput(or1), Gate_getOutput(or2));
    Gate* and2 = new_AndGate(Gate_getOutput(and1), Gate_getOutput(or3));

    Value** inputs = new_Value_array(3);
    inputs[0] = inx;
    inputs[1] = iny;
    inputs[2] = inz;

    Gate** gates = new_Gate_array(8);
    gates[0] = not1;
    gates[1] = not2;
    gates[2] = not3;
    gates[3] = or1;
    gates[4] = or2;
    gates[5] = or3;
    gates[6] = and1;
    gates[7] = and2;

    Value** outputs = new_Value_array(1);
    outputs[0] = Gate_getOutput(and2);

    return new_Circuit(3, inputs, 1, outputs, 8, gates);
}

Circuit* build_circuit_D(){
    Value* inx = new_Value(false);
    Value* iny = new_Value(false);
    Value* inz = new_Value(false);

    Gate* not1 = new_Inverter(inx);
    Gate* not2 = new_Inverter(iny);
    Gate* not3 = new_Inverter(inz);

    Gate* and1 = new_And3Gate(inx,iny,inz);                         ///x,y,z
    Gate* and2 = new_And3Gate(Gate_getOutput(not1),iny,inz);    ///~x,y,z
    Gate* and3 = new_And3Gate(inx, Gate_getOutput(not2),inz);   ///x,~y,z
    Gate* and4 = new_And3Gate(inx,iny, Gate_getOutput(not3));   ///x,y,~z
    Gate* and5 = new_And3Gate(Gate_getOutput(not1), Gate_getOutput(not2),inz); ///~x,~y,z
    Gate* and6 = new_And3Gate(Gate_getOutput(not1),iny, Gate_getOutput(not3)); ///~x,y,~z
    Gate* and7 = new_And3Gate(inx, Gate_getOutput(not2), Gate_getOutput(not3));///x,~y,~z

    Gate* or1  = new_Or4Gate(Gate_getOutput(and5), Gate_getOutput(and6),Gate_getOutput(and7), Gate_getOutput(and4)); ///{5,6,7,4}
    Gate* or2  = new_Or4Gate(Gate_getOutput(and5), Gate_getOutput(and2),Gate_getOutput(and3), Gate_getOutput(and1)); ///{5,2,3,1}

    Value** inputs = new_Value_array(3);
    inputs[0] = inx;
    inputs[1] = iny;
    inputs[2] = inz;

    Gate** gates = new_Gate_array(12);
    gates[0] = not1;
    gates[1] = not2;
    gates[2] = not3;

    gates[3] = and1;
    gates[4] = and2;
    gates[5] = and3;
    gates[6] = and4;
    gates[7] = and5;
    gates[8] = and6;
    gates[9] = and7;

    gates[10] = or1;
    gates[11] = or2;

    Value** outputs = new_Value_array(2);
    outputs[0] = Gate_getOutput(or1);
    outputs[1] = Gate_getOutput(or2);

    return new_Circuit(3, inputs, 2, outputs, 12, gates);

}

int main() {

    printf("Please enter circuilt 1,2,3 or 4(extra credit): ");
    scanf("%c",strinput);

        if(strcmp(strinput,"1") ==0)
        {
            printf("Circuit A:\nX Y    OUT\n--------------------\n");
            test_all(build_circuit_A());
        }
        else if(strcmp(strinput,"2") ==0)
        {
            printf("\nCircuit B:\nX Y    OUT\n---------------------\n");
            test_all(build_circuit_B());
        }
        else if(strcmp(strinput,"3") ==0)
        {
            printf("\nCircuit C:\nX Y Z    OUT\n----------------------\n");
            test_all(build_circuit_C());
        }
        else if(strcmp(strinput,"4") ==0)
        {
            printf("\nCircuit D(extra credit):\nX Y Z    F G\n---------------------------\n");
            test_all(build_circuit_D());
        }else{
            printf("(Invalid input)");
        }
}

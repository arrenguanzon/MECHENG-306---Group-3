#include "FSM.h"

FSM::FSM()
{
    state = IDLE;
}

void FSM::update()
{
    switch (state)
    {
        case IDLE:
            updateIdle();
            break;

        case HOMING:
            updateHoming();
            break;

        case MOVING:
            updateMoving();
            break;

        case FAULT:
            updateFault();
            break;
    }
}

void FSM::updateIdle()
{
    // Implement the logic for the IDLE state
}

void FSM::updateHoming()
{
    
}

void FSM::updateMoving()
{
    // Implement the logic for the MOVING state
}

void FSM::updateFault()
{
    // Implement the logic for the FAULT state
}




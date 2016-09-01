#ifndef ADDITIONAL_CREATE_H
#define ADDITIONAL_CREATE_H

#include "./renderer.h"
#include "./manager.h"

CApplicationRendererAdditionalBase* Create_ApplicationRendererAdditional()
{
    return NULL;
}

CApplicationManagerAdditionalBase* Create_ApplicationManagerAdditional(CAscApplicationManager* pManager)
{
    return NULL;
}

#endif // ADDITIONAL_CREATE_H

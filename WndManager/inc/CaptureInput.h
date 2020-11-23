#pragma once

#include "Types.h"

#include <memory>

class CaptureInput
{
    std::shared_ptr<CaptureInput> m_pPrevCapture;
    bool                          m_fCaptured {false};

public:
    CaptureInput() = default;
    virtual ~CaptureInput() {InputRelease();}

    virtual input_t EventProc(input_t code) {return code;}
    virtual bool  InputCapture();
    virtual bool  InputRelease();

    bool IsInputCaptured() {return m_fCaptured;}
};

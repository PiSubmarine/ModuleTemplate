#include <gtest/gtest.h>
#include "PiSubmarine/ModuleTemplate/ModuleTemplate.h"

namespace PiSubmarine::ModuleTemplate
{
    TEST(ModuleTemplateTest, GetReturnCode)
    {
        ASSERT_EQ(ModuleTemplate::GetReturnCode(), 0);
    }
}
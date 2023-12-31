import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil


class TestCase(TestBase):
    def test(self):
        self.build()
        self.createTestTarget()

        # Empty structs/classes have size 1 in C++.
        self.expect_expr("sizeof(Empty) == sizeof_empty", result_value="true")
        self.expect_expr(
            "sizeof(EmptyClass) == sizeof_empty_class", result_value="true"
        )
        self.expect_expr(
            "sizeof(EmptyClassAligned) == sizeof_empty_class_aligned",
            result_value="true",
        )
        self.expect_expr(
            "sizeof(ClassEmptyMember) == sizeof_class_empty_member", result_value="true"
        )
        self.expect_expr("sizeof(SingleMember) == sizeof_single", result_value="true")
        self.expect_expr(
            "sizeof(SingleMemberClass) == sizeof_single_class", result_value="true"
        )
        self.expect_expr("sizeof(PaddingMember) == sizeof_padding", result_value="true")
        self.expect_expr(
            "sizeof(PaddingMemberClass) == sizeof_padding_class", result_value="true"
        )

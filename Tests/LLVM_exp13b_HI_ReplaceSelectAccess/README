The HI_ReplaceSelectAccess pass is used to handle those accesses which have more than one access target.

This pass transform the pattern like:

%4 = xxxxx (Pointer-Type)
%5 = xxxxx (Pointer-Type)
%6 = select i1 %sw, i32* %4, i32* %5
%7 = load i32* %6 ...

into:

%4 = xxxxx (Pointer-Type)
%5 = xxxxx (Pointer-Type)
%44 = load i32* %4 ...
%55 = load i32* %5 ...
%7 = select i1 %sw, i32 %44, i32 %55

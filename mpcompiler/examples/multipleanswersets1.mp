[common signature]
predicate:p1/1;
predicate:x/0;

[belief base]
name:bb1;
mapping: "p1(a).";

[belief base]
name:bb2;
mapping: "x v p1(y).";

[merging plan]
{
	bb2
}

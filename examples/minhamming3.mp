[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;

[belief base]
name: bb1;
mapping: "a. -b.";

[belief base]
name: bb2;
mapping: "-a. b.";

[merging plan]
{
	operator: minhamming;
	constraint: ":- a, not b.";
	{bb1};
	{bb2};
}

[common signature]
predicate:xyz/0;
predicate:foo/0;

[belief base]
name:bb1;
mapping: "x. y.";

[merging plan]
{
	operator: testop1;
	{
		bb1
	};
	{
		bb1
	};
}

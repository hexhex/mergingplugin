[common signature]
predicate:x/0;
predicate:y/0;

[belief base]
name:bb1;
mapping: "x. y.";

[merging plan]
{
	operator: testop2;
	{
		bb1
	};
	{
		operator: testop2;
		{
			bb1
		};
		{
			bb1
		};
	};
}

[common signature]
predicate: a/0;
predicate: b/0;
predicate: p/2;
predicate: q/2;

[belief base]
name: kb1;
mapping: "
	a.
	-b.
	p(x,y).
	-q(x,y).
";

[merging plan]
{
	operator: union;
	{
		kb1
	};
	{
		kb1
	};
}

[common signature]
predicate: a/0;
predicate: b/0;
predicate: p/1;

[belief base]
name: kb1;
mapping: "
	p(a).
	-p(b).
";

[belief base]
name: kb2;
mapping: "
	b.
";

[merging plan]
{
	operator: union;
	{
		kb1
	};
	{
		kb2
	};
}

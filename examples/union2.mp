[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;
predicate: d/0;

[belief base]
name: kb1;
mapping: "
	a.
";

[belief base]
name: kb2;
mapping: "
	b v d.
	c :- b.
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

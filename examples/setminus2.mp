[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;

[belief base]
name: kb1;
mapping: "
	b.
";

[belief base]
name: kb2;
mapping: "
	b.
	c :- b.
	a :- c.
";

[merging plan]
{
	operator: setminus;
	{
		kb2
	};
	{
		kb1
	};
}

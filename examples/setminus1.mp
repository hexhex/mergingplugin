[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;

[belief base]
name: kb1;
mapping: "a. b. c.";

[belief base]
name: kb2;
mapping: "b.";

[merging plan]
{
	operator: setminus;
	{
		kb1
	};
	{
		kb2
	};
}

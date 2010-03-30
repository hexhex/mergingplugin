[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;
predicate: d/0;
predicate: e/0;
predicate: p/1;

[belief base]
name: bb1;
mapping: "a. b.";

[belief base]
name: bb2;
mapping: "b. c.";

[belief base]
name: bb3;
mapping: "a. c.";

[merging plan]
{
	operator: dalal;
	constraint: ":- a, b, c.";
	{bb1};
	{bb2};
	{bb3};
}

[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;
predicate: d/0;
predicate: e/0;
predicate: p/1;

[belief base]
name: bb1;
mapping: "a. -b. c.";

[belief base]
name: bb2;
mapping: "-a. d.";

[merging plan]
{
	operator: dalal;
	{bb1};
	{bb2};
	{bb3};
}

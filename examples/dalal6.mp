[common signature]
predicate: p/1;

[belief base]
name: bb1;
mapping: "p(a). a.";

[belief base]
name: bb2;
mapping: "p(b). b.";

[merging plan]
{
	operator: dalal;
	constraint: ":- p(X), p(Y), X != Y.";
	constraint: ":- a, b.";
	ignore: "a,b";
	{bb1};
	{bb2};
}

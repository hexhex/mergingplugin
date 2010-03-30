[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;
predicate: d/0;
predicate: e/0;
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

[common signature]
predicate: a/0;
predicate: b/0;
predicate: c/0;
predicate: d/0;
predicate: e/0;
predicate: p/1;

[belief base]
name: bb1;
mapping: "a. p(x).";

[belief base]
name: bb2;
mapping: "-a. -p(x).";

[belief base]
name: bb3;
mapping: "-a.";

[merging plan]
{
	operator: dbo;
	constraint: "";
	aggregate: "sum";
	weights: "1,1,1";
	{bb1};
	{bb2};
	{bb3};
}

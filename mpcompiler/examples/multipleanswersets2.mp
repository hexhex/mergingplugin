[common signature]
predicate:a/0;
predicate:x/0;
predicate:y/0;
predicate:z/0;

[belief base]
name:bb1;
mapping: "p1(a).";

[belief base]
name:bb2;
mapping: "x v y. a :- x. z :- y.";

[merging plan]
{
	bb2
}

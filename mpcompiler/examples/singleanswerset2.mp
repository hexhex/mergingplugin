[common signature]
predicate:x/0;
predicate:y/0;

[belief base]
name:bb1;
mapping: "a. b.";

[belief base]
name:bb2;
mapping: "x. y :- x.";

[merging plan]
{
	bb2
}

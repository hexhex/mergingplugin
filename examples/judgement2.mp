[common signature]
predicate: v/0;
predicate: b/0;
predicate: d/0;

[belief base]
name: individual1;
mapping: "v. b. d.";

[belief base]
name: individual2;
mapping: "v. -b. -d.";

[belief base]
name: individual3;
mapping: "-v. b. -d.";

[merging plan]
{
	operator: majorityselection;
	majorityOf: "d";
	{
		operator: minhamming;
		constraint: ":- v, b, not d.";
		constraint: ":- v, not b, d.";
		constraint: ":- not v, b, d.";
		constraint: ":- not v, not b, d.";
		{individual1};
		{individual2};
		{individual3};
	};
}

[common signature]
predicate: data/5;
predicate: schema/5;

[belief base]
name: source1;
mapping: "
schema(firstname, lastname, salary, sex).
data(homer,simpson,1900, male).
data(marge,simpson,1000, female).
";

[belief base]
name: source2;
mapping: "
schema(firstname, lastname, salary, age).
data(carl,carlson,2500,37).
data(homer,simpson,1900,38).
";

[belief base]
name: source3;
mapping: "
schema(firstname, lastname, salary).
data(monty,burns,120000,108).
";

[merging plan]
{
	operator: relationmerging;
	schema: "firstname,lastname,salary,sex,age";
	default: "sex=unknown";
	default: "age=unknown";
	key: "firstname,lastname";
	{source1};
	{source2};
	{source3};
}

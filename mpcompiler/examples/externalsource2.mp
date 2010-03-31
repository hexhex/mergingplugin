[common signature]
predicate:a/0;
predicate:b/0;
predicate:c/0;
predicate:x/0;
predicate:y/0;

[belief base]
name:bb1;
source: "externalsource1.hex";
args: "--dlv --plugindir=../../src/";

[belief base]
name:bb2;
source: "externalsource2.hex";
args: "--dlv --plugindir=../../src/";

[merging plan]
{
	operator: testop2;
	{
		bb1
	};
	{
		bb2
	};
}

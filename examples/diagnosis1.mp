[common signature]
predicate:ab/1;

[belief base]
name:juror1;
dlvmapping: "";
dlvargs: "-FRmin fulladder.dl abnormal1.hyp fault.obs";
args: "--plugindir=../src/.libs/ ";

[belief base]
name:juror2;
dlvmapping: "";
dlvargs: "-FRmin fulladder.dl abnormal2.hyp fault.obs";
args: "--plugindir=../src/.libs/";

[belief base]
name:juror3;
dlvmapping: "";
dlvargs: "-FRmin fulladder.dl abnormal3.hyp fault.obs";
args: "--plugindir=../src/.libs/";

[merging plan]
{
	operator: dalal;
	constraintfile: "fulladder.dl";
	constraintfile: "fault.obs";
	penalize: "ignoring";
	maxint: "3";
	aggregate:"sum";
	{juror1};
	{juror2};
	{juror3};
}

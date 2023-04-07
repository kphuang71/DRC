#pragma once
//
#include<math.h>

#define SAMPLE_INTERVAL_MULTIPLY_2_2 4.988662131519275e-05


/**< drc for single channel */
class drc
{
public:
	drc();
	void setTypical();
	void setByPassCure();
	void setCure(double ntx, double nty, double etx, double ety, double ctx, double cty, double ltx, double lty);
	void setTimePara(double ta, double tr, double tm); // in miniseconds
	void setParasRate();
	double xGained(double in);
private:
	double xPEAK[2];	//0 for current value, 1 for 1-delay value  
	double xRMS[2];	//�¾�RMSֵ 0 for current value, 1 for 1-delay value
	double xGN[2];	//�¾�����ֵ 0 for current value, 1 for 1-delay value
	double NT[2];	//DRC���������޺�������, ��0�������������룬��1��������������
	double ET[2];	//DRC����չ���޺�������, ��0�������������룬��1�������������� 
	double CT[2];	//DRC��ѹ�����޺�������, ��0�������������룬��1�������������� 
	double LT[2];	//DRC���޷����޺�������, ��0�������������룬��1�������������� 
	double ParasTime[3];	//DRC����Ӧ��ʱ��������ֱ����һ��DRC��AT/RT/TAV 
	double ParasRate[3];	//DRC����Ӧ��б�ʣ� [0]Ϊ������������չ���������б��NT~ET��[1]Ϊ��չ������ѹ�����������б��ET~CT��[2]Ϊѹ���������޷����������б��CT~LT ;

};



drc::drc() // constructor, for initial purpose
{
	drc::setTypical();
	xPEAK[0] = 0; xPEAK[1] = 0;
	xRMS[0] = 0; xRMS[1] = 0;
	xGN[0] = 1; xGN[1] = 1;
}

void drc::setTimePara(double ta, double tr, double tm)
{
	ParasTime[0] = 1.0 - exp(-SAMPLE_INTERVAL_MULTIPLY_2_2 / ta);	//����TA 
	ParasTime[1] = 1.0 - exp(-SAMPLE_INTERVAL_MULTIPLY_2_2 / tr);	//����TR 
	ParasTime[2] = 1.0 - exp(-SAMPLE_INTERVAL_MULTIPLY_2_2 / tm);	//����TAV 
}

void drc::setParasRate()
{
	ParasRate[0] = (ET[1] - NT[1]) / (ET[0] -NT[0]);	//������������չ����֮���߶�б�� 
	ParasRate[1] = (CT[1] - ET[1]) / (CT[0] - ET[0]);	//��չ������ѹ������֮���߶�б�� 
	ParasRate[2] = (LT[1] - CT[1]) / (LT[0] - CT[0]);	//ѹ���������޷�����֮���߶�б�� 
}


void drc::setTypical() // a typical drc 
{
	NT[0] = -60; NT[1] = -120;
	ET[0] = -40; ET[1] = -34;
	CT[0] = -20; CT[1] = -17;
	LT[0] = -5; LT[1] = -8;
	drc::setParasRate();
	drc::setTimePara(10, 10, 20);
}

void drc::setByPassCure() // a typical drc 
{
	NT[0] = -60; NT[1] = -60;
	ET[0] = -40; ET[1] = -40;
	CT[0] = -20; CT[1] = -20;
	LT[0] = -5; LT[1] = -5;
	drc::setParasRate();
}

void drc::setCure(double ntx, double nty, double etx, double ety, double ctx, double cty, double ltx, double lty)
{
	NT[0] = ntx; NT[1] = nty;
	ET[0] = etx; ET[1] = ety;
	CT[0] = ctx; CT[1] = cty;
	LT[0] = ltx; LT[1] = lty;
	drc::setParasRate();
}



double drc::xGained(double in)
{
	double LogPeak, LogRms, fn = 1.0;
	//��������ľ���ֵ���ֵ��ֵ�Ƚϣ����·�ֵ 
	if (fabs(in) > xPEAK[1]) {
		xPEAK[0] = (1.0 - ParasTime[0]) * xPEAK[1] + ParasTime[0] * fabs(in);
	}
	else {
		xPEAK[0] = (1.0 - ParasTime[1]) * xPEAK[1];
	}	
	//����xRMSֵ 
	xRMS[0] = (1.0 - ParasTime[2]) * xRMS[1] + ParasTime[2] * in * in;


	//����ֵ��xRMS������ֵת��Ϊ�ֱ�ֵ 
	LogPeak = 20 * log10(xPEAK[0]);
	LogRms = 10 * log10(xRMS[0]);

	//����������������������� 
	if (LogPeak > LT[0]) {
		fn = pow(10.0, (LT[1] - LogPeak) * 0.05);
	}
	else if (LogRms < NT[0]) {
		fn = 0.0;
	}
	else if (LogRms < ET[0]) {
		fn = pow(10.0, (NT [1] + (LogRms - NT[0]) * ParasRate[0] - LogRms) * 0.05);
	}
	else if (LogRms < CT[0]) {
		fn = pow(10.0, (ET[1] + (LogRms - ET[0]) * ParasRate[1] - LogRms) * 0.05);
	}
	else {
		fn = pow(10.0, (CT[1] + (LogRms - CT[0]) * ParasRate[2] - LogRms) * 0.05);
	}

	//�������������������ͨ�˲���ƽ������ 
	if (fn > xGN[1]) {
		xGN[0] = (1.0 - ParasTime [0]) * xGN[1] + ParasTime [0] * fn;
	}
	else {
		xGN[0] = (1.0 - ParasTime [1]) * xGN[1] + ParasTime [1] * fn;
	}

	// updating
	xPEAK[1] = xPEAK[0]; 
	xRMS[1] = xRMS[0];
	xGN[1] = xGN[0]; 

	return xGN[0];	//���ص�ǰ���������� 
}


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
	double xRMS[2];	//新旧RMS值 0 for current value, 1 for 1-delay value
	double xGN[2];	//新旧增益值 0 for current value, 1 for 1-delay value
	double NT[2];	//DRC的噪声门限横纵坐标, 列0代表横坐标或输入，列1代表纵坐标或输出
	double ET[2];	//DRC的扩展门限横纵坐标, 列0代表横坐标或输入，列1代表纵坐标或输出 
	double CT[2];	//DRC的压缩门限横纵坐标, 列0代表横坐标或输入，列1代表纵坐标或输出 
	double LT[2];	//DRC的限幅门限横纵坐标, 列0代表横坐标或输入，列1代表纵坐标或输出 
	double ParasTime[3];	//DRC所对应的时间参数，分别代表一个DRC的AT/RT/TAV 
	double ParasRate[3];	//DRC所对应的斜率， [0]为噪声门限与扩展门限区间的斜率NT~ET，[1]为扩展门限与压缩门限区间的斜率ET~CT，[2]为压缩门限与限幅门限区间的斜率CT~LT ;

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
	ParasTime[0] = 1.0 - exp(-SAMPLE_INTERVAL_MULTIPLY_2_2 / ta);	//计算TA 
	ParasTime[1] = 1.0 - exp(-SAMPLE_INTERVAL_MULTIPLY_2_2 / tr);	//计算TR 
	ParasTime[2] = 1.0 - exp(-SAMPLE_INTERVAL_MULTIPLY_2_2 / tm);	//计算TAV 
}

void drc::setParasRate()
{
	ParasRate[0] = (ET[1] - NT[1]) / (ET[0] -NT[0]);	//噪声门限与扩展门限之间线段斜率 
	ParasRate[1] = (CT[1] - ET[1]) / (CT[0] - ET[0]);	//扩展门限与压缩门限之间线段斜率 
	ParasRate[2] = (LT[1] - CT[1]) / (LT[0] - CT[0]);	//压缩门限与限幅门限之间线段斜率 
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
	//根据输入的绝对值与峰值旧值比较，求新峰值 
	if (fabs(in) > xPEAK[1]) {
		xPEAK[0] = (1.0 - ParasTime[0]) * xPEAK[1] + ParasTime[0] * fabs(in);
	}
	else {
		xPEAK[0] = (1.0 - ParasTime[1]) * xPEAK[1];
	}	
	//求新xRMS值 
	xRMS[0] = (1.0 - ParasTime[2]) * xRMS[1] + ParasTime[2] * in * in;


	//将峰值和xRMS的线性值转换为分贝值 
	LogPeak = 20 * log10(xPEAK[0]);
	LogRms = 10 * log10(xRMS[0]);

	//根据输入所在区间计算增益 
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

	//增益修正，或者增益低通滤波，平滑处理 
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

	return xGN[0];	//返回当前所得新增益 
}


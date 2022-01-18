#pragma once

#include "ui_qtOpencv.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>

#include <opencv2/opencv.hpp>

class qtOpencv : public QWidget
{
    Q_OBJECT

public:
    qtOpencv(QWidget *parent = Q_NULLPTR);

	void initConnect();
	void initUi();

	// 使用label 显示mat
	void labelDisplayMat(QLabel *label, cv::Mat &mat);
	// 生成颜色
	void confirmColor(cv::Vec3b &color, cv::RNG &rng);
public slots:
	void exportOriginPicture();
	void createTarget();
	void saveTarget();
private:
	cv::Mat mat2Simples(cv::Mat& image);
private:
    Ui::qtOpencvClass ui;
	// 图片路径
	QString m_importFilename;
	// 生成的mat对象
	cv::Mat m_targetMat;
};

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

	// ʹ��label ��ʾmat
	void labelDisplayMat(QLabel *label, cv::Mat &mat);
	// ������ɫ
	void confirmColor(cv::Vec3b &color, cv::RNG &rng);
public slots:
	void exportOriginPicture();
	void createTarget();
	void saveTarget();
private:
	cv::Mat mat2Simples(cv::Mat& image);
private:
    Ui::qtOpencvClass ui;
	// ͼƬ·��
	QString m_importFilename;
	// ���ɵ�mat����
	cv::Mat m_targetMat;
};

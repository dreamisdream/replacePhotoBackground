#include "qtOpencv.h"
#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace ml;

const int pixelsUnit = 96;

qtOpencv::qtOpencv(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
	initUi();
	initConnect();
}

void qtOpencv::initConnect()
{
	// 导入图片
	connect(this->ui.importPicBtn, &QPushButton::clicked, this, &qtOpencv::exportOriginPicture);
	// 背景替换
	connect(this->ui.createTargetBtn, &QPushButton::clicked, this, &qtOpencv::createTarget);
	// 导出效果图
	connect(this->ui.exportTargetBtn, &QPushButton::clicked, this, &qtOpencv::saveTarget);

}

void qtOpencv::initUi() 
{
	this->setFixedSize(600, 400);
	this->ui.originPicLabel->setText(QString("Origin Picture").toUtf8());
	this->ui.targetPicLabel->setText("Target Picture");

	this->ui.originPicLabel->setFixedSize(2.5*pixelsUnit, 3.3*pixelsUnit);
	this->ui.targetPicLabel->setFixedSize(2.5*pixelsUnit, 3.3*pixelsUnit);

	QStringList strList;
	strList << "Blue" << "Green" << "Red ";
	this->ui.backgroundSelect->addItems(strList);
	for (int i = 0; i < strList.size(); ++i) {
		static_cast<QStandardItemModel*>(this->ui.backgroundSelect->model())->item(i)->setTextAlignment(Qt::AlignCenter);
	} 
	
}

Mat qtOpencv::mat2Simples(Mat& image) {
	int w = image.cols;
	int h = image.rows;
	Mat points(w*h, image.channels(), CV_32F, Scalar(10));

	int index = 0;
	for (int row = 0; row < h; row++) {
		for (int col = 0; col < w; col++) {
			index = row * w + col;
			Vec3b bgr = image.at<Vec3b>(row, col);
			points.at<float>(index, 0) = static_cast<int>(bgr[0]);
			points.at<float>(index, 1) = static_cast<int>(bgr[1]);
			points.at<float>(index, 2) = static_cast<int>(bgr[2]);
		}
	}
	return points;
}

void qtOpencv::createTarget()
{
	QFile file(m_importFilename);
	Mat src;
	if (!file.open(QFile::ReadOnly))
	{
		cout << "读取失败" << endl;
	}
	else {
		QByteArray ba = file.readAll();
		src = imdecode(vector<char>(ba.begin(), ba.end()), 1);
	}

	if (src.empty()) {
		cout << "Can't open image." << endl;
		return ;
	}

	//组装数据
	Mat points = mat2Simples(src);

	//KMeans运行
	int numCluster = 4;
	Mat labels, centers;
	TermCriteria criteria = TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1);
	kmeans(points, numCluster, labels, criteria, 3, KMEANS_PP_CENTERS, centers);

	//去背景+遮罩生成
	Mat mask = Mat::zeros(src.size(), CV_8UC1); // 遮罩层
	int index = src.rows * 2 + 2;// 取左上角的（2,2）坐标为背景颜色
	int c_index = labels.at<int>(index, 0);
	int height = src.rows;
	int width = src.cols;
	Mat dst;
	src.copyTo(dst);
	for (int row = 0; row < height; ++row) {
		for (int col = 0; col < width; ++col) {
			index = row * width + col;
			int label = labels.at<int>(index, 0);
			if (label == c_index) {
				//背景区域
				dst.at<Vec3b>(row, col)[0] = 0;
				dst.at<Vec3b>(row, col)[1] = 0;
				dst.at<Vec3b>(row, col)[2] = 0;
				mask.at<uchar>(row, col) = 0;
			} else {
				mask.at<uchar>(row, col) = 255;
			}
		}
	}

	// 形态学腐蚀+高斯模糊
	Mat k = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	erode(mask, mask, k);
	GaussianBlur(mask, mask, Size(3, 3), 0, 0);

	// 通道混合
	RNG rng(12345);
	Vec3b color;
	confirmColor(color,rng);

	Mat result(src.size(), src.type());

	double w = 0.0;
	int b = 0, g = 0, r = 0;
	int b1 = 0, g1 = 0, r1 = 0;
	int b2 = 0, g2 = 0, r2 = 0;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			int m = mask.at<uchar>(row, col);
			if (m == 255) { //前景
				result.at<Vec3b>(row, col) = src.at<Vec3b>(row, col);
			} else if (m == 0) { //背景
				result.at<Vec3b>(row, col) = color;
			} else {
				w = m / 255;
				b1 = src.at<Vec3b>(row, col)[0];//前景
				g1 = src.at<Vec3b>(row, col)[1];
				r1 = src.at<Vec3b>(row, col)[2];

				b2 = color[0];//背景
				g2 = color[1];
				r2 = color[2];

				b = b1 * w + b2 * (1.0 - w);
				g = g1 * w + g2 * (1.0 - w);
				r = r1 * w + r2 * (1.0 - w);

				result.at<Vec3b>(row, col)[0] = b;
				result.at<Vec3b>(row, col)[1] = g;
				result.at<Vec3b>(row, col)[2] = r;
			}
		}
	}

	this->labelDisplayMat(this->ui.targetPicLabel, result);
	this->m_targetMat = result;
}

void qtOpencv::saveTarget()
{
	if (m_importFilename.isEmpty())
		return;

	int index = m_importFilename.toStdString().find_last_of("/");
	char path[128] = {};
	sprintf(path, "%s/%d_%s",
		QDir::currentPath().toStdString().c_str(),
		this->ui.backgroundSelect->currentIndex(),
		m_importFilename.toStdString().substr(index + 1).c_str()
	);

	auto result = QMessageBox::information(NULL, "File save path", path, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	 // 确认后保存成功
	if (result == QMessageBox::Yes) {
		imwrite(path, this->m_targetMat);
	}
}

void qtOpencv::labelDisplayMat(QLabel *label, cv::Mat &mat)
{
	cv::Mat Rgb;
	QImage Img;
	if (mat.channels() == 3)//RGB Img
	{
		cv::cvtColor(mat, Rgb, COLOR_BGR2RGB);//颜色空间转换
		Img = QImage((const uchar*)(Rgb.data), Rgb.cols, Rgb.rows, Rgb.cols * Rgb.channels(), QImage::Format_RGB888);
	}
	else//Gray Img
	{
		Img = QImage((const uchar*)(mat.data), mat.cols, mat.rows, mat.cols*mat.channels(), QImage::Format_Indexed8);
	}

	Img.scaled(this->ui.originPicLabel->size(), Qt::KeepAspectRatio);//把图片
	label->setScaledContents(true);
	label->setPixmap(QPixmap::fromImage(Img));
}

void qtOpencv::confirmColor(cv::Vec3b& color, cv::RNG &rng)
{
	int index = this->ui.backgroundSelect->currentIndex();
	if (index == 0) {
		color[0] = rng.uniform(0, 255);
		color[1] = rng.uniform(0, 0);
		color[2] = rng.uniform(0, 0);
	} else if (index == 1) {
		color[0] = rng.uniform(0, 0);
		color[1] = rng.uniform(0, 255);
		color[2] = rng.uniform(0, 0);
	} else if (index == 2) {
		color[0] = rng.uniform(0, 0);
		color[1] = rng.uniform(0, 0);
		color[2] = rng.uniform(0, 255);
	} else if(index == 3) {
		color[0] = rng.uniform(0, 255);
		color[1] = rng.uniform(0, 255);
		color[2] = rng.uniform(0, 255);
	}

	return;
}

void qtOpencv::exportOriginPicture()
{
	m_importFilename = QFileDialog::getOpenFileName(this, tr("Open Image"), "../qtOpencv/img/", tr("Image Files (*.png *.jpg *.bmp)"));
	QImage *img = new QImage; //新建一个image对象

	img->load(m_importFilename); //将图像资源载入对象img，注意路径，可点进图片右键复制路径

	// 等比例缩放
	img->scaled(this->ui.originPicLabel->size(), Qt::KeepAspectRatio);//把图片
	this->ui.originPicLabel->setScaledContents(true);

	this->ui.originPicLabel->setPixmap(QPixmap::fromImage(*img));
	
}
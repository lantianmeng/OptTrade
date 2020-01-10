#pragma once
#ifndef JMG_IMAGE_WIDGET
#define JMG_IMAGE_WIDGET

#include <QWidget>

class QLabel;
class JmgImageWidget :public QWidget
{
	Q_OBJECT
public:
	JmgImageWidget(QWidget *parent = nullptr);
	~JmgImageWidget();

	void addImage(QImage img);
	void clear();
	int size();
	void show();
	void setSelected(int index);
signals:
	void selected_image(int index);
protected:
	virtual void mousePressEvent(QMouseEvent *);
private:
	QGridLayout *toolLayout;
	QList<QLabel*> imageList;          //显示每个task中的image，目前主要用于jmg格式中的图片列表
};
#endif // !JMG_IMAGE_WIDGET

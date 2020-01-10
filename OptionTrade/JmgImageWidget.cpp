#include "stdafx.h"
#include "JmgImageWidget.h"

JmgImageWidget::JmgImageWidget(QWidget *parent)
{
	this->setParent(parent);

	toolLayout = new QGridLayout(this);
	toolLayout->setAlignment(Qt::AlignTop);
	setLayout(toolLayout);
}

JmgImageWidget::~JmgImageWidget()
{

}

void JmgImageWidget::addImage(QImage img)
{
	QPixmap pix = QPixmap::fromImage(img);
	QLabel *show_image_unit = new QLabel(this);
	show_image_unit->setPixmap(pix.scaled(QSize(128, 128)));
	imageList.push_back(show_image_unit);
}

void JmgImageWidget::clear()
{
	for (auto img : imageList)
	{
		imageList.removeOne(img);
	}
}

int JmgImageWidget::size() 
{
	return imageList.size(); 
}

void JmgImageWidget::show()
{
	//先清除
	QLayoutItem *child;
	while ((child = toolLayout->takeAt(0)) != 0)
	{
		toolLayout->removeWidget(child->widget());
		child->widget()->setParent(nullptr);
		delete child;
	}

	//再添加
	for (auto img : imageList)
	{
		toolLayout->addWidget(img);
	}
}

void JmgImageWidget::mousePressEvent(QMouseEvent *event)
{
	for (auto& img : imageList)
	{
		if (img->geometry().contains(this->mapFromGlobal(event->globalPos())))
		{
			int index = imageList.indexOf(img);
			imageList[index]->setStyleSheet("QLabel{border:2px solid rgb(0, 255, 0);}");
			emit selected_image(index);
			//qDebug() << imageList.indexOf(img);
		}
		else
		{
			img->setStyleSheet("QLabel{border:none;}");
		}
	}
	
}

void JmgImageWidget::setSelected(int index)
{
	if (index >= 0 && index < imageList.size())
	{
		imageList[index]->setStyleSheet("QLabel{border:2px solid rgb(0, 255, 0);}");
		
		for (int i = 0; i < imageList.size(); ++i)
		{
			if (i == index)
			{
				imageList[i]->setStyleSheet("QLabel{border:2px solid rgb(0, 255, 0);}");
				emit selected_image(i);
			}
			else
			{
				imageList[i]->setStyleSheet("QLabel{border:none;}");
			}
		}
	}
}

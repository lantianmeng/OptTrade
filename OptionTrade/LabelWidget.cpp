#include "stdafx.h"
#include "LabelWidget.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QCompleter>

LabelWidget::LabelWidget(QStringList labels, QDialog *parent):QDialog(parent)
{
	layout = new QVBoxLayout(this);

	input_label = new QLineEdit;
	input_label->setPlaceholderText("Enter object label");

	button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal,this);
	connect(button_box, &QDialogButtonBox::accepted, [this] {if(input_label->text().size() > 0) accept(); });
	connect(button_box, &QDialogButtonBox::rejected, [this] {reject(); });

	labelList = new QListWidget;
	connect(labelList, &QListWidget::currentItemChanged, [this]() 
	{
		QListWidgetItem *item = labelList->currentItem();
		if(item)
		input_label->setText(labelList->currentItem()->text()); 
	});
	labelList->addItems(labels);
	labelList->sortItems();

	layout->addWidget(input_label);
	layout->addWidget(button_box);
	layout->addWidget(labelList);

	completer = new QCompleter();
	completer->setCompletionMode(QCompleter::InlineCompletion);
	completer->setFilterMode(Qt::MatchStartsWith);
	completer->setModel(labelList->model());
	input_label->setCompleter(completer);
}

LabelWidget::~LabelWidget()
{

}

int LabelWidget::popUp()
{
	int ret_code = exec();
	if (ret_code == QDialog::Accepted)
	{
		QString input_text = input_label->text();
		
		QList<QListWidgetItem *> items = labelList->findItems(input_text, Qt::MatchFixedString);
		if (items.size() > 0)
		{
			labelList->setCurrentItem(items[0]);
		}
	}

	return ret_code;
}

QString LabelWidget::GetLabelText()
{
	return  input_label->text();
}

void LabelWidget::addLabelHistory(QString text)
{
	QList<QListWidgetItem *> temp_itemlist = labelList->findItems(text, Qt::MatchExactly);
	if (temp_itemlist.size() == 0)
	{
		QListWidgetItem *temp_item = new QListWidgetItem(text);
		labelList->addItem(temp_item);
		labelList->sortItems();
	}
}

void LabelWidget::addItems(QStringList labels)
{
	labelList->addItems(labels);
	labelList->sortItems();
}

void LabelWidget::clear()
{
	labelList->clear();
}

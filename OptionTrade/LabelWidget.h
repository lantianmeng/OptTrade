#pragma once
#ifndef LABEL_WIDGET_H
#define LABEL_WIDGET_H

#include <QDialog>

class QCompleter;
class QVBoxLayout;
class QLineEdit;
class QDialogButtonBox;
class QListWidget;

class LabelWidget:public QDialog
{
	Q_OBJECT
public:
	LabelWidget(QStringList labels, QDialog *parent = nullptr);
	~LabelWidget();

	//返回值：1：accepted 0：rejected
	int popUp();
	QString GetLabelText();

	void addLabelHistory(QString text);
	void addItems(QStringList labels);
	void clear();
private:
	QVBoxLayout *layout;
	QLineEdit *input_label;
	QDialogButtonBox *button_box;
	QListWidget *labelList;
    QCompleter *completer;
};
#endif // !LABEL_WIDGET_H

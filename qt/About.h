
#ifndef ABOUT_20130520_H_
#define ABOUT_20130520_H_

#include <QDialog>
#include "ui_About.h"

class About final : public QDialog {
	Q_OBJECT
	friend class Pretendo;
public:
	About(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	~About() override;
	
private:
	Ui::About ui_;
};

#endif

#include "qlightboxdialog.h"

#include <QAbstractScrollArea>
#include <QEventLoop>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>

namespace {
	/**
	 * @brief Создать объект анимации появления/скрытия для заданного виджета
	 */
	static QPropertyAnimation* createOpacityAnimation(QWidget* _forWidget) {
		QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(_forWidget);

		QPropertyAnimation* opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity");
		opacityAnimation->setDuration(120);
		opacityAnimation->setEasingCurve(QEasingCurve::OutCirc);
		opacityAnimation->setStartValue(0);
		opacityAnimation->setEndValue(1);

		opacityEffect->setOpacity(0);
		_forWidget->setGraphicsEffect(opacityEffect);

		return opacityAnimation;
	}
}


QLightBoxDialog::QLightBoxDialog(QWidget *parent, bool _followToHeadWidget, bool _isContentStretchable) :
	QLightBoxWidget(parent, _followToHeadWidget),
	m_title(new QLabel(this)),
	m_centralWidget(0),
	m_isContentStretchable(_isContentStretchable),
	m_execResult(Rejected)
{
	initView();
	initConnections();
}

int QLightBoxDialog::exec()
{
	m_execResult = Rejected;

	m_title->setText(windowTitle());
	if (m_title->text().isEmpty()) {
		m_title->hide();
	}

#ifndef NO_ANIMATIONS
	animateShow();
#else
	show();
#endif

	focusedOnExec()->setFocus();

	QEventLoop dialogEventLoop;
	connect(this, SIGNAL(accepted()), &dialogEventLoop, SLOT(quit()));
	connect(this, SIGNAL(rejected()), &dialogEventLoop, SLOT(quit()));
	connect(this, SIGNAL(finished(int)), &dialogEventLoop, SLOT(quit()));
	dialogEventLoop.exec();

#ifndef NO_ANIMATIONS
	animateHide();
#else
	hide();
#endif

	return m_execResult;
}

void QLightBoxDialog::accept()
{
	m_execResult = Accepted;
	emit accepted();
}

void QLightBoxDialog::reject()
{
	m_execResult = Rejected;
	emit rejected();
}

void QLightBoxDialog::done(int _result)
{
	m_execResult = _result;
	emit finished(_result);
}

bool QLightBoxDialog::event(QEvent* _event)
{
	bool result = true;
	bool needHandle = true;
	if (_event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(_event);
		if (keyEvent->key() == Qt::Key_Enter
			|| keyEvent->key() == Qt::Key_Return) {
			accept();
			needHandle = false;
		} else if (keyEvent->key() == Qt::Key_Escape) {
			reject();
			needHandle = false;
		}
	}

	if (needHandle) {
		result = QLightBoxWidget::event(_event);
	}

	return result;
}

void QLightBoxDialog::initView()
{
	m_title->setProperty("lightBoxDialogTitle", true);
	m_title->setWordWrap(true);
#ifdef Q_OS_MAC
	m_title->setAlignment(Qt::AlignHCenter);
#endif

	if (layout() != 0) {
		m_centralWidget = new QFrame(this);
		m_centralWidget->setProperty("lightBoxDialogCentralWidget", true);
		m_centralWidget->setMinimumSize(minimumSize());
		m_centralWidget->setMaximumSize(maximumSize());
		m_centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		QLayout* centralWidgetLayout = layout();
		m_centralWidget->setLayout(centralWidgetLayout);

		setMinimumSize(QSize(0, 0));

		QGridLayout* newLayout = new QGridLayout;
		newLayout->setContentsMargins(QMargins());
		newLayout->setSpacing(0);
		newLayout->addWidget(m_title, 1, 1);
		newLayout->addWidget(m_centralWidget, 2, 1);
		newLayout->setRowStretch(0, 1);
		if (m_isContentStretchable) {
			newLayout->setRowStretch(2, 8);
		}
		newLayout->setRowStretch(3, 1);
		newLayout->setColumnStretch(0, 1);
		newLayout->setColumnStretch(2, 1);
		setLayout(newLayout);
	}
}

void QLightBoxDialog::initConnections()
{
}

QWidget* QLightBoxDialog::focusedOnExec() const
{
	return m_centralWidget;
}

void QLightBoxDialog::animateShow()
{
	animate(true);
}

void QLightBoxDialog::animateHide()
{
	animate(false);
}

void QLightBoxDialog::animate(bool _forward)
{
	//
	// Показываем виджет, если нужно
	//
	if (_forward) {
		show();
	}

	//
	// Приходится создавать несколько дополнительных анимаций помимо самого диалога
	// так же для всех viewport'ов областей прокрутки, т.к. к ним не применяется эффект прозрачности
	//
	QParallelAnimationGroup opacityAnimationGroup;
	opacityAnimationGroup.addAnimation(::createOpacityAnimation(this));
	foreach (QAbstractScrollArea* scrollArea, findChildren<QAbstractScrollArea*>()) {
		opacityAnimationGroup.addAnimation(::createOpacityAnimation(scrollArea->viewport()));
	}
	opacityAnimationGroup.setDirection(_forward ? QPropertyAnimation::Forward : QPropertyAnimation::Backward);
	opacityAnimationGroup.start(QAbstractAnimation::DeleteWhenStopped);

	//
	// Ожидаем завершения анимации
	//
	QEventLoop animationEventLoop;
	connect(&opacityAnimationGroup, SIGNAL(finished()), &animationEventLoop, SLOT(quit()));
	animationEventLoop.exec();

	//
	// Скрываем виджет, если нужно
	//
	if (!_forward) {
		hide();
	}

	//
	// Удаляем эффект анимации, т.к. иногда из-за него коряво отрисовываются некоторые виджеты
	//
	setGraphicsEffect(0);
	foreach (QAbstractScrollArea* scrollArea, findChildren<QAbstractScrollArea*>()) {
		scrollArea->viewport()->setGraphicsEffect(0);
		scrollArea->viewport()->repaint();
	}
}

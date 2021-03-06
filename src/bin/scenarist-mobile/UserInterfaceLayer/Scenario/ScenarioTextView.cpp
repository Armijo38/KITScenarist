#include "ScenarioTextView.h"
#include "ui_ScenarioTextView.h"

#include <BusinessLayer/ScenarioDocument/ScenarioTemplate.h>
#include <BusinessLayer/ScenarioDocument/ScenarioTextDocument.h>
#include <BusinessLayer/ScenarioDocument/ScenarioTemplate.h>
#include <BusinessLayer/ScenarioDocument/ScenarioTextBlockInfo.h>
#include <BusinessLayer/Chronometry/ChronometerFacade.h>

#include <UserInterfaceLayer/ScenarioTextEdit/ScenarioTextEdit.h>
#include <UserInterfaceLayer/ScenarioTextEdit/ScenarioTextEditHelpers.h>

#include <3rd_party/Helpers/ScrollerHelper.h>
#include <3rd_party/Helpers/ShortcutHelper.h>
#include <3rd_party/Helpers/StyleSheetHelper.h>
#include <3rd_party/Widgets/ScalableWrapper/ScalableWrapper.h>
#include <3rd_party/Widgets/QLightBoxWidget/qlightboxinputdialog.h>
#include <3rd_party/Widgets/WAF/Animation.h>

#include <QCryptographicHash>
#include <QKeyEvent>
#include <QScrollBar>
#include <QScroller>

using UserInterface::ScenarioTextView;
using UserInterface::ScenarioTextEdit;
using BusinessLogic::ScenarioTemplateFacade;
using BusinessLogic::ScenarioTemplate;
using BusinessLogic::ScenarioBlockStyle;
using BusinessLogic::ScenarioTextBlockInfo;

namespace {
	/**
	 * @brief Название свойства, под которым кнопки хранят задаваемые стили
	 */
	const char* BLOCK_STYLE = "propertyScenarioBlockStyle";

	/**
	 * @brief Получить хэш текста
	 */
	static QByteArray textMd5Hash(const QString& _text) {
		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(_text.toUtf8());
		return hash.result();
	}
}


ScenarioTextView::ScenarioTextView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::ScenarioTextView),
	m_editor(new ScenarioTextEdit(this)),
	m_editorWrapper(new ScalableWrapper(m_editor, this))
{
	m_ui->setupUi(this);

	initView();
	initConnections();
	initStyleSheet();
}

ScenarioTextView::~ScenarioTextView()
{
	delete m_ui;
}

QWidget* ScenarioTextView::toolbar() const
{
	return m_ui->toolbar;
}

void ScenarioTextView::setScenarioName(const QString& _name)
{
	m_ui->scenarioName->setText(_name);
}

BusinessLogic::ScenarioTextDocument* ScenarioTextView::scenarioDocument() const
{
	return qobject_cast<BusinessLogic::ScenarioTextDocument*>(m_editor->document());
}

void ScenarioTextView::setScenarioDocument(BusinessLogic::ScenarioTextDocument* _document, bool _isDraft)
{
	removeEditorConnections();

	m_editor->setScenarioDocument(_document);
	m_editor->setWatermark(_isDraft ? tr("DRAFT") : QString::null);
	if (_document != 0) {
		m_lastTextMd5Hash = ::textMd5Hash(_document->toPlainText());
	}

	initEditorConnections();
}

//void ScenarioTextView::setDuration(const QString& _duration)
//{
//	m_duration->setText(_duration);
//}

//void ScenarioTextView::setCountersInfo(const QString& _counters)
//{
//	m_countersInfo->setText(_counters);
//}

void ScenarioTextView::setShowScenesNumbers(bool _show)
{
	m_editor->setShowSceneNumbers(_show);
}

void ScenarioTextView::setHighlightCurrentLine(bool _highlight)
{
	m_editor->setHighlightCurrentLine(_highlight);
}

void ScenarioTextView::setAutoReplacing(bool _replacing)
{
	m_editor->setAutoReplacing(_replacing);
}

void ScenarioTextView::setUsePageView(bool _use)
{
	//
	// Установка постраничного режима так же тянет за собой ряд настроек
	//
	QMarginsF pageMargins(15, 5, 5, 5);
	Qt::Alignment pageNumbersAlign;
	if (_use) {
		pageMargins = ScenarioTemplateFacade::getTemplate().pageMargins();
		pageNumbersAlign = ScenarioTemplateFacade::getTemplate().numberingAlignment();
	}

	m_editor->setUsePageMode(_use);
	m_editor->setPageMargins(pageMargins);
	m_editor->setPageNumbersAlignment(pageNumbersAlign);

	//
	// В дополнение установим шрифт по умолчанию для документа (шрифтом будет рисоваться нумерация)
	//
	m_editor->document()->setDefaultFont(
		ScenarioTemplateFacade::getTemplate().blockStyle(ScenarioBlockStyle::Action).font());
}

void ScenarioTextView::setUseSpellChecker(bool _use)
{
	m_editor->setUseSpellChecker(_use);
}

void ScenarioTextView::setSpellCheckLanguage(int _language)
{
	m_editor->setSpellCheckLanguage((SpellChecker::Language)_language);
}

void ScenarioTextView::setTextEditColors(const QColor& _textColor, const QColor& _backgroundColor)
{
	m_editor->viewport()->setStyleSheet(QString("color: %1; background-color: %2;").arg(_textColor.name(), _backgroundColor.name()));
	m_editor->setStyleSheet(QString("#scenarioEditor { color: %1; }").arg(_textColor.name()));
}

void ScenarioTextView::setTextEditZoomRange(qreal _zoomRange)
{
	m_editorWrapper->setZoomRange(_zoomRange);
}

int ScenarioTextView::cursorPosition() const
{
	return m_editor->textCursor().position();
}

void ScenarioTextView::setCursorPosition(int _position)
{
	//
	// Устанавливаем позицию курсора
	//
	QTextCursor cursor = m_editor->textCursor();

	//
	// Если это новая позиция
	//
	if (cursor.position() != _position) {
		//
		// Устанавливаем реальную позицию
		//
		cursor.setPosition(_position);
		m_editor->setTextCursor(cursor);
		m_editor->ensureCursorVisible();
		//
		// Прокручиваем вниз, чтобы курсор стоял в верху экрана
		//
		m_editor->verticalScrollBar()->setValue(m_editor->verticalScrollBar()->maximum());

		//
		// Возвращаем курсор в поле зрения
		//
		m_editor->ensureCursorVisible();
		m_editorWrapper->setFocus();
	}
	//
	// Если нужно обновить в текущей позиции курсора просто имитируем отправку сигнала
	//
	else {
		emit m_editor->cursorPositionChanged();
	}
}

void ScenarioTextView::addItem(int _position, int _type, const QString& _header,
	const QColor& _color, const QString& _description)
{
	QTextCursor cursor = m_editor->textCursor();
	cursor.beginEditBlock();

	cursor.setPosition(_position);
	m_editor->setTextCursor(cursor);
	ScenarioBlockStyle::Type type = (ScenarioBlockStyle::Type)_type;
	//
	// Если в позиции пустой блок, изменим его
	//
	if (cursor.block().text().isEmpty()) {
		m_editor->changeScenarioBlockType(type);
	}
	//
	// В противном случае добавим новый
	//
	else {
		m_editor->addScenarioBlock(type);
	}

	//
	// Устанавливаем текст в блок
	//
	m_editor->insertPlainText(_header);

	//
	// Устанавливаем цвет и описание
	//
	cursor = m_editor->textCursor();
	QTextBlockUserData* textBlockData = cursor.block().userData();
	ScenarioTextBlockInfo* info = dynamic_cast<ScenarioTextBlockInfo*>(textBlockData);
	if (info == 0) {
		info = new ScenarioTextBlockInfo;
	}
	info->setColors(_color.name());
	info->setDescription(_description);
	cursor.block().setUserData(info);

	//
	// Если это группирующий блок, то вставим и закрывающий текст
	//
	if (ScenarioTemplateFacade::getTemplate().blockStyle(type).isEmbeddableHeader()) {
		cursor = m_editor->textCursor();
		cursor.movePosition(QTextCursor::NextBlock);
		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		cursor.insertText(Helpers::footerText(_header));
	}

	cursor.endEditBlock();
}

void ScenarioTextView::removeText(int _from, int _to)
{
	QTextCursor cursor = m_editor->textCursor();
	cursor.beginEditBlock();

	//
	// Стираем текст
	//
	cursor.setPosition(_from);
	cursor.setPosition(_to, QTextCursor::KeepAnchor);
	cursor.removeSelectedText();

	//
	// Если остаётся пустой блок, стираем его тоже
	//
	if (cursor.block().text().isEmpty()) {
		//
		// Стирать необходимо через имитацию удаления редактором,
		// для корректного обновления модели сцен
		//
		QKeyEvent* event = 0;
		if (cursor.atStart()) {
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
		} else {
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
		}
		QApplication::sendEvent(m_editor, event);
	}

	cursor.endEditBlock();
}

void ScenarioTextView::updateStylesElements()
{
	//
	// Обновить выпадающий список стилей сценария
	//
	m_textTypes.clear();
	initStylesCombo();
}

void ScenarioTextView::updateShortcuts()
{
	m_editor->updateShortcuts();
}

void ScenarioTextView::setAdditionalCursors(const QMap<QString, int>& _cursors)
{
	m_editor->setAdditionalCursors(_cursors);
}

void ScenarioTextView::setCommentOnly(bool _isCommentOnly)
{
	m_editor->setReadOnly(_isCommentOnly);
}

void ScenarioTextView::aboutUndo()
{
	m_editor->undoReimpl();
}

void ScenarioTextView::aboutRedo()
{
	m_editor->redoReimpl();
}

void ScenarioTextView::resizeEvent(QResizeEvent* _event)
{
    QWidget::resizeEvent(_event);

    aboutUpdateTextStyle();
}

void ScenarioTextView::aboutUpdateTextStyle()
{
	//
	// Определим текущий стиль
	//
	ScenarioBlockStyle::Type currentType = m_editor->scenarioBlockType();
	if (currentType == ScenarioBlockStyle::TitleHeader) {
		currentType = ScenarioBlockStyle::Title;
	} else if (currentType == ScenarioBlockStyle::SceneGroupFooter) {
		currentType = ScenarioBlockStyle::SceneGroupHeader;
	} else if (currentType == ScenarioBlockStyle::FolderFooter) {
		currentType = ScenarioBlockStyle::FolderHeader;
	}

	//
	// Включим необходимую кнопку стиля
	//
	switch (currentType) {
		case ScenarioBlockStyle::SceneHeading: m_ui->sceneHeadingStyle->setChecked(true); break;
		case ScenarioBlockStyle::SceneCharacters: m_ui->sceneCharactersStyle->setChecked(true); break;
		case ScenarioBlockStyle::Action: m_ui->actionStyle->setChecked(true); break;
		case ScenarioBlockStyle::Character: m_ui->characterStyle->setChecked(true); break;
		case ScenarioBlockStyle::Dialogue: m_ui->dialogueStyle->setChecked(true); break;
		case ScenarioBlockStyle::Parenthetical: m_ui->parentheticalStyle->setChecked(true); break;
		default: m_ui->textStyle->setChecked(true); break;
	}

	//
	// Обновим отображение кнопок
	//
	QButtonGroup* buttonGroup = m_ui->textStyle->group();
	foreach (QAbstractButton* styleButton, buttonGroup->buttons()) {
		if (styleButton->isChecked()) {
			styleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

			//
			// Отображаем полное название стиля, только если место позволяет
			//
			static const int MIN_HEIGHT_FOR_FULL_NAMES = StyleSheetHelper::dpToPx(420);
			if (width() >= MIN_HEIGHT_FOR_FULL_NAMES) {
				static const bool BEAUTIFY = true;
				styleButton->setText(
					ScenarioBlockStyle::typeName(currentType, BEAUTIFY)
					+ (styleButton == m_ui->textStyle ? " ..." : ""));
			}
			//
			// Если места мало, то используем сокращённые названия стилей блоков
			//
			else {
				styleButton->setText(
					ScenarioBlockStyle::shortTypeName(currentType)
					+ (styleButton == m_ui->textStyle ? " ..." : ""));
			}
		} else {
			styleButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			styleButton->setText(
				styleButton == m_ui->textStyle
				? "..."
				: ScenarioBlockStyle::shortTypeName((ScenarioBlockStyle::Type)styleButton->property(BLOCK_STYLE).toInt()));
		}
	}
}

void ScenarioTextView::aboutChangeTextStyle()
{
	ScenarioBlockStyle::Type selectedType = ScenarioBlockStyle::Undefined;

	//
	// Если это нажатие на кнопку с заданным стилем, то просто применяем его
	//
	if (sender()->property(BLOCK_STYLE).isValid()) {
		selectedType = (ScenarioBlockStyle::Type)sender()->property(BLOCK_STYLE).toInt();
	}
	//
	// В противном случае покажем диалог для выбора стиля, который нужно установить
	//
	else {
		//
		// Показываем диалог выбора стиля для блока
		//
		QStringList types;
		QMap<QString, int> typesMap;
		{
			QPair<QString, int> type;
			foreach (type, m_textTypes) {
				types.append(type.first);
				typesMap.insert(type.first, type.second);
			}
		}
		QString currentTypeName = m_ui->textStyle->text();
		currentTypeName = currentTypeName.remove("...").simplified();
		const QString selectedTypeName =
				QLightBoxInputDialog::getItem(this, tr("Choose block type"), types, currentTypeName);

		//
		// Если стиль выбран
		//
		if (!selectedTypeName.isEmpty()) {
			selectedType = (ScenarioBlockStyle::Type)typesMap.value(selectedTypeName);
		}
	}

	//
	// Меняем стиль блока, если это возможно
	//
	if (selectedType != ScenarioBlockStyle::Undefined) {
		m_editor->changeScenarioBlockType(selectedType);
		m_editorWrapper->setFocus();
	}
	//
	// Если нет, то просто обновляем текущий стиль
	//
	else {
		aboutUpdateTextStyle();
	}
}

void ScenarioTextView::aboutCursorPositionChanged()
{
	emit cursorPositionChanged(m_editor->textCursor().position());
}

void ScenarioTextView::aboutTextChanged()
{
	QByteArray currentTextMd5Hash = textMd5Hash(m_editor->document()->toPlainText());
	if (m_lastTextMd5Hash != currentTextMd5Hash) {
		emit textChanged();

		m_lastTextMd5Hash = currentTextMd5Hash;
	}
}

void ScenarioTextView::aboutStyleChanged()
{
	emit textChanged();
}

void ScenarioTextView::initView()
{
	m_ui->scenarioName->setElideMode(Qt::ElideRight);
	m_ui->textEditContainer->addWidget(m_editorWrapper);

	m_editor->setTextSelectionEnable(false);
    m_editor->setUseCompleter(false);
	ScrollerHelper::addScroller(m_editor);

	m_ui->sceneHeadingStyle->setFocusPolicy(Qt::NoFocus);
	m_ui->sceneHeadingStyle->setProperty(BLOCK_STYLE, ScenarioBlockStyle::SceneHeading);
	m_ui->sceneCharactersStyle->setFocusPolicy(Qt::NoFocus);
	m_ui->sceneCharactersStyle->setProperty(BLOCK_STYLE, ScenarioBlockStyle::SceneCharacters);
	m_ui->actionStyle->setFocusPolicy(Qt::NoFocus);
	m_ui->actionStyle->setProperty(BLOCK_STYLE, ScenarioBlockStyle::Action);
	m_ui->characterStyle->setFocusPolicy(Qt::NoFocus);
	m_ui->characterStyle->setProperty(BLOCK_STYLE, ScenarioBlockStyle::Character);
	m_ui->dialogueStyle->setFocusPolicy(Qt::NoFocus);
	m_ui->dialogueStyle->setProperty(BLOCK_STYLE, ScenarioBlockStyle::Dialogue);
	m_ui->parentheticalStyle->setFocusPolicy(Qt::NoFocus);
	m_ui->parentheticalStyle->setProperty(BLOCK_STYLE, ScenarioBlockStyle::Parenthetical);

	QButtonGroup* stylesGroup = new QButtonGroup(this);
	stylesGroup->addButton(m_ui->sceneHeadingStyle);
	stylesGroup->addButton(m_ui->sceneCharactersStyle);
	stylesGroup->addButton(m_ui->actionStyle);
	stylesGroup->addButton(m_ui->characterStyle);
	stylesGroup->addButton(m_ui->dialogueStyle);
	stylesGroup->addButton(m_ui->parentheticalStyle);
	stylesGroup->addButton(m_ui->textStyle);
}

void ScenarioTextView::initStylesCombo()
{
	ScenarioTemplate style = ScenarioTemplateFacade::getTemplate();
	const bool BEAUTIFY_NAME = true;

	QList<ScenarioBlockStyle::Type> types;
	types << ScenarioBlockStyle::Title
		  << ScenarioBlockStyle::Note
		  << ScenarioBlockStyle::Transition
		  << ScenarioBlockStyle::NoprintableText
		  << ScenarioBlockStyle::SceneGroupHeader
		  << ScenarioBlockStyle::FolderHeader;

	foreach (ScenarioBlockStyle::Type type, types) {
		if (style.blockStyle(type).isActive()) {
			m_textTypes.append(QPair<QString, int>(ScenarioBlockStyle::typeName(type, BEAUTIFY_NAME), type));
		}
	}
}

void ScenarioTextView::initConnections()
{
	connect(m_ui->navigator, &QToolButton::clicked, this, &ScenarioTextView::showNavigatorClicked);

	//
	// Настраиваем применения стилей
	//
    connect(m_ui->sceneHeadingStyle, &QPushButton::clicked, this, &ScenarioTextView::aboutChangeTextStyle);
    connect(m_ui->sceneCharactersStyle, &QPushButton::clicked, this, &ScenarioTextView::aboutChangeTextStyle);
    connect(m_ui->actionStyle, &QPushButton::clicked, this, &ScenarioTextView::aboutChangeTextStyle);
    connect(m_ui->characterStyle, &QPushButton::clicked, this, &ScenarioTextView::aboutChangeTextStyle);
    connect(m_ui->dialogueStyle, &QPushButton::clicked, this, &ScenarioTextView::aboutChangeTextStyle);
    connect(m_ui->parentheticalStyle, &QPushButton::clicked, this, &ScenarioTextView::aboutChangeTextStyle);
    connect(m_ui->textStyle, &QPushButton::clicked, this, &ScenarioTextView::aboutChangeTextStyle);

    //
    // Настраиваем отображение панелей в зависимости от того открыта ли клавиатура
	//
	connect(QApplication::inputMethod(), &QInputMethod::visibleChanged, [=] {
		if (isVisible()) {
			while (QApplication::inputMethod()->isAnimating()) {
				QApplication::processEvents();
			}

            QWidget* topToolbar = m_ui->toolbar->parentWidget()->parentWidget()->parentWidget();
            QVBoxLayout* layout = m_ui->mainLayout;
            layout->takeAt(layout->indexOf(m_ui->editingToolbar));
            if (QApplication::inputMethod()->isVisible()) {
                topToolbar->hide();
                layout->insertWidget(0, m_ui->editingToolbar);
            } else {
                topToolbar->show();
                layout->addWidget(m_ui->editingToolbar);
            }

            m_editor->ensureCursorVisible();
		}
	});

	initEditorConnections();
}

void ScenarioTextView::initEditorConnections()
{
	connect(m_editor, &ScenarioTextEdit::currentStyleChanged, this, &ScenarioTextView::aboutUpdateTextStyle);
	connect(m_editor, &ScenarioTextEdit::cursorPositionChanged, this, &ScenarioTextView::aboutUpdateTextStyle);
	connect(m_editor, &ScenarioTextEdit::cursorPositionChanged, this, &ScenarioTextView::aboutCursorPositionChanged);
	connect(m_editor, &ScenarioTextEdit::textChanged, this, &ScenarioTextView::aboutTextChanged);
	connect(m_editor, &ScenarioTextEdit::styleChanged, this, &ScenarioTextView::aboutStyleChanged);
	connect(m_editor, &ScenarioTextEdit::reviewChanged, this, &ScenarioTextView::textChanged);
	connect(m_editorWrapper, &ScalableWrapper::zoomRangeChanged, this, &ScenarioTextView::zoomRangeChanged);
}

void ScenarioTextView::removeEditorConnections()
{
	disconnect(m_editor, &ScenarioTextEdit::currentStyleChanged, this, &ScenarioTextView::aboutUpdateTextStyle);
	disconnect(m_editor, &ScenarioTextEdit::cursorPositionChanged, this, &ScenarioTextView::aboutUpdateTextStyle);
	disconnect(m_editor, &ScenarioTextEdit::cursorPositionChanged, this, &ScenarioTextView::aboutCursorPositionChanged);
	disconnect(m_editor, &ScenarioTextEdit::textChanged, this, &ScenarioTextView::aboutTextChanged);
	disconnect(m_editor, &ScenarioTextEdit::styleChanged, this, &ScenarioTextView::aboutStyleChanged);
	disconnect(m_editor, &ScenarioTextEdit::reviewChanged, this, &ScenarioTextView::textChanged);
	disconnect(m_editorWrapper, &ScalableWrapper::zoomRangeChanged, this, &ScenarioTextView::zoomRangeChanged);
}

void ScenarioTextView::initStyleSheet()
{
	m_ui->toolbar->setProperty("toolbar", true);
	m_ui->scenarioName->setProperty("toolbar", true);
    m_ui->sceneHeadingStyle->setProperty("flat-black", true);
    m_ui->sceneCharactersStyle->setProperty("flat-black", true);
    m_ui->actionStyle->setProperty("flat-black", true);
    m_ui->characterStyle->setProperty("flat-black", true);
    m_ui->dialogueStyle->setProperty("flat-black", true);
    m_ui->parentheticalStyle->setProperty("flat-black", true);
    m_ui->textStyle->setProperty("flat-black", true);
	m_ui->search->hide();
	m_ui->menu->hide();
}

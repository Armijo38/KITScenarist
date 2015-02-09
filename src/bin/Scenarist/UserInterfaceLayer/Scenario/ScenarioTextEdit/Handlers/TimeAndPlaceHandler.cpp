#include "TimeAndPlaceHandler.h"

#include "../ScenarioTextEdit.h"

#include <BusinessLayer/ScenarioDocument/ScenarioTextBlockParsers.h>

#include <Domain/Place.h>
#include <Domain/Location.h>
#include <Domain/ScenarioDay.h>
#include <Domain/Time.h>

#include <DataLayer/DataStorageLayer/StorageFacade.h>
#include <DataLayer/DataStorageLayer/PlaceStorage.h>
#include <DataLayer/DataStorageLayer/LocationStorage.h>
#include <DataLayer/DataStorageLayer/ScenarioDayStorage.h>
#include <DataLayer/DataStorageLayer/TimeStorage.h>

#include <QKeyEvent>
#include <QTextBlock>

using namespace Domain;
using namespace DataStorageLayer;
using namespace KeyProcessingLayer;
using namespace BusinessLogic;
using UserInterface::ScenarioTextEdit;


TimeAndPlaceHandler::TimeAndPlaceHandler(ScenarioTextEdit* _editor) :
	StandardKeyHandler(_editor)
{
}

void TimeAndPlaceHandler::handleEnter(QKeyEvent*)
{
	//
	// Получим необходимые значения
	//
	// ... курсор в текущем положении
	QTextCursor cursor = editor()->textCursor();
	// ... блок текста в котором находится курсор
	QTextBlock currentBlock = cursor.block();
	// ... текст до курсора
	QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
	// ... текст после курсора
	QString cursorForwardText = currentBlock.text().mid(cursor.positionInBlock());
	// ... текущая секция
	TimeAndPlaceParser::Section currentSection =
			TimeAndPlaceParser::section(cursorBackwardText);


	//
	// Обработка
	//
	if (editor()->isCompleterVisible()) {
		//! Если открыт подстановщик

		//
		// Вставить выбранный вариант
		//
		editor()->applyCompletion();

		//
		// Обновим курсор, т.к. после автозавершения он смещается
		//
		cursor = editor()->textCursor();

		//
		// Дописать необходимые символы
		//
		switch (currentSection) {
			case TimeAndPlaceParser::SectionPlace: {
				cursor.insertText(". ");
				break;
			}

			default: {
				break;
			}
		}

		//
		// Покажем подсказку, если это возможно
		//
		handleOther();

	} else {
		//! Подстановщик закрыт

		if (cursor.hasSelection()) {
			//! Есть выделение

			//
			// Ни чего не делаем
			//
		} else {
			//! Нет выделения

			if (cursorBackwardText.isEmpty()
				&& cursorForwardText.isEmpty()) {
				//! Текст пуст

				//
				// Ни чего не делаем
				//
			} else {
				//! Текст не пуст

				//
				// Сохраним параметры сцены
				//
				storeSceneParameters();

				if (cursorBackwardText.isEmpty()) {
					//! В начале блока

					//
					// Вставка блока заголовка перед собой
					//
					editor()->addScenarioBlock(ScenarioBlockStyle::TimeAndPlace);
				} else if (cursorForwardText.isEmpty()) {
					//! В конце блока

					//
					// Вставка блока описания действия
					//
//					editor()->addScenarioBlock(ScenarioBlockStyle::Action);
					editor()->addScenarioBlock(jumpForEnter(ScenarioBlockStyle::TimeAndPlace));
				} else {
					//! Внутри блока

					//
					// Вставка блока описания действия
					//
					editor()->addScenarioBlock(ScenarioBlockStyle::Action);
				}
			}
		}
	}
}

void TimeAndPlaceHandler::handleTab(QKeyEvent*)
{
	//
	// Получим необходимые значения
	//
	// ... курсор в текущем положении
	QTextCursor cursor = editor()->textCursor();
	// ... блок текста в котором находится курсор
	QTextBlock currentBlock = cursor.block();
	// ... текст до курсора
	QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
	// ... текст после курсора
	QString cursorForwardText = currentBlock.text().mid(cursor.positionInBlock());


	//
	// Обработка
	//
	if (editor()->isCompleterVisible()) {
		//! Если открыт подстановщик

		//
		// Работаем аналогично нажатию ENTER
		//
		handleEnter();
	} else {
		//! Подстановщик закрыт

		if (cursor.hasSelection()) {
			//! Есть выделение

			//
			// Ни чего не делаем
			//
		} else {
			//! Нет выделения

			if (cursorBackwardText.isEmpty()
				&& cursorForwardText.isEmpty()) {
				//! Текст пуст

				//
				// Если строка пуста, то сменить стиль на описание действия
				//
				editor()->changeScenarioBlockType(ScenarioBlockStyle::Action);
			} else {
				//! Текст не пуст

				if (cursorBackwardText.isEmpty()) {
					//! В начале блока

					//
					// Ни чего не делаем
					//
				} else if (cursorForwardText.isEmpty()) {
					//! В конце блока

					//
					// Если нет времени, то добавление " - " и отображение подсказки
					//
					if (TimeAndPlaceParser::timeName(cursorBackwardText).isEmpty()) {
						//
						// Добавим необходимый текст в зависимости от того, что ввёл пользователь
						//
						if (cursorBackwardText.endsWith(" -")) {
							cursor.insertText(" ");
						} else if (cursorBackwardText.endsWith(" ")) {
							cursor.insertText("- ");
						} else {
							cursor.insertText(" - ");
						}

						//
						// Отображение подсказки
						//
						handleOther();
					}

					//
					// В противном случае перейдём к блоку участников сцены
					//
					else {
						//
						// Сохраним параметры сцены
						//
						storeSceneParameters();

						//
						// А затем вставим блок
						//
//						editor()->addScenarioBlock(ScenarioBlockStyle::SceneCharacters);
						editor()->addScenarioBlock(jumpForTab(ScenarioBlockStyle::TimeAndPlace));
					}
				} else {
					//! Внутри блока

					//
					// Ни чего не делаем
					//
				}
			}
		}
	}
}

void TimeAndPlaceHandler::handleOther(QKeyEvent*)
{
	//
	// Получим необходимые значения
	//
	// ... курсор в текущем положении
	QTextCursor cursor = editor()->textCursor();
	// ... блок текста в котором находится курсор
	QTextBlock currentBlock = cursor.block();
	// ... текст блока
	QString currentBlockText = currentBlock.text();
	// ... текст до курсора
	QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
	// ... текущая секция
	TimeAndPlaceParser::Section currentSection = TimeAndPlaceParser::section(cursorBackwardText);


	//
	// Получим модель подсказок для текущей секции и выведем пользователю
	//
	QAbstractItemModel* sectionModel = 0;
	//
	// ... в соответствии со введённым в секции текстом
	//
	QString sectionText;

	switch (currentSection) {
		case TimeAndPlaceParser::SectionPlace: {
			sectionModel = StorageFacade::placeStorage()->all();
			sectionText = TimeAndPlaceParser::placeName(currentBlockText);
			break;
		}

		case TimeAndPlaceParser::SectionLocation: {
			sectionModel = StorageFacade::locationStorage()->all();
			sectionText = TimeAndPlaceParser::locationName(currentBlockText);
			break;
		}

		case TimeAndPlaceParser::SectionScenarioDay: {
			sectionModel = StorageFacade::scenarioDayStorage()->all();
			sectionText = TimeAndPlaceParser::scenarioDayName(currentBlockText);
			break;
		}

		case TimeAndPlaceParser::SectionTime: {
			sectionModel = StorageFacade::timeStorage()->all();
			sectionText = TimeAndPlaceParser::timeName(currentBlockText);
			break;
		}

		default: {
			break;
		}
	}

	//
	// Дополним текст
	//
	editor()->complete(sectionModel, sectionText);
}

void TimeAndPlaceHandler::storeSceneParameters() const
{
	if (editor()->storeDataWhenEditing()) {
		//
		// Получим необходимые значения
		//
		// ... курсор в текущем положении
		QTextCursor cursor = editor()->textCursor();
		// ... блок текста в котором находится курсор
		QTextBlock currentBlock = cursor.block();
		// ... текст блока
		QString currentBlockText = currentBlock.text();

		//
		// Сохраняем время
		//
		QString placeName = TimeAndPlaceParser::placeName(currentBlockText);
		StorageFacade::placeStorage()->storePlace(placeName);

		//
		// Сохраняем локацию
		//
		QString locationName = TimeAndPlaceParser::locationName(currentBlockText);
		StorageFacade::locationStorage()->storeLocation(locationName);

		//
		// Сохраняем место
		//
		QString timeName = TimeAndPlaceParser::timeName(currentBlockText);
		StorageFacade::timeStorage()->storeTime(timeName);

		//
		// Сохраняем сценарный день
		//
		QString scenarioDayName = TimeAndPlaceParser::scenarioDayName(currentBlockText);
		StorageFacade::scenarioDayStorage()->storeScenarioDay(scenarioDayName);
	}
}

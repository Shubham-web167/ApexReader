import re
import os

pdfview_h_path = r'e:\Programing\AcrobatKiller\include\PdfView.h'
with open(pdfview_h_path, 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace('#include <QSet>', '#include <QSet>\n#include <QUndoStack>')
content = content.replace('    void setDocument(PdfDocument* doc);', '    void setDocument(PdfDocument* doc);\n\n    QUndoStack* undoStack() const { return m_undoStack; }')
content = content.replace('    QGraphicsScene* m_scene;', '    QGraphicsScene* m_scene;\n    QUndoStack* m_undoStack;')

with open(pdfview_h_path, 'w', encoding='utf-8') as f:
    f.write(content)


pdfview_cpp_path = r'e:\Programing\AcrobatKiller\src\PdfView.cpp'
with open(pdfview_cpp_path, 'r', encoding='utf-8') as f:
    content = f.read()

undo_command_code = r'''#include <QUndoCommand>

class AddAnnotationCommand : public QUndoCommand {
public:
    AddAnnotationCommand(QGraphicsScene* scene, QGraphicsItem* item, QUndoCommand* parent = nullptr)
        : QUndoCommand("Add Annotation", parent), m_scene(scene), m_item(item) {
    }

    void undo() override {
        m_scene->removeItem(m_item);
    }

    void redo() override {
        m_scene->addItem(m_item);
    }

private:
    QGraphicsScene* m_scene;
    QGraphicsItem* m_item;
};

class RemoveAnnotationCommand : public QUndoCommand {
public:
    RemoveAnnotationCommand(QGraphicsScene* scene, QGraphicsItem* item, QUndoCommand* parent = nullptr)
        : QUndoCommand("Remove Annotation", parent), m_scene(scene), m_item(item) {
    }

    void undo() override {
        m_scene->addItem(m_item);
    }

    void redo() override {
        m_scene->removeItem(m_item);
    }

private:
    QGraphicsScene* m_scene;
    QGraphicsItem* m_item;
};
'''

content = content.replace('#include <QScrollBar>', '#include <QScrollBar>\n' + undo_command_code)
content = content.replace('m_currentZoom = 1.0f;', 'm_currentZoom = 1.0f;\n    m_undoStack = new QUndoStack(this);')

# Fix adding annotations to use UndoStack
content = content.replace('m_scene->addItem(m_currentDrawing);', 'm_undoStack->push(new AddAnnotationCommand(m_scene, m_currentDrawing));')

erase_code_old = r'''        QGraphicsItem* item = scene()->itemAt(mapToScene(event->pos()), transform());
        if (item && item->data(0).toString() == "user_annotation") {
            scene()->removeItem(item);
            delete item;
        }'''
erase_code_new = r'''        QGraphicsItem* item = scene()->itemAt(mapToScene(event->pos()), transform());
        if (item && item->data(0).toString() == "user_annotation") {
            m_undoStack->push(new RemoveAnnotationCommand(m_scene, item));
        }'''
content = content.replace(erase_code_old, erase_code_new)

textnote_old = r'''        m_scene->addItem(textItem);
        textItem->setFocus();'''
textnote_new = r'''        m_undoStack->push(new AddAnnotationCommand(m_scene, textItem));
        textItem->setFocus();'''
content = content.replace(textnote_old, textnote_new)

highlight_old = r'''            m_scene->addItem(newHighlightItem);'''
highlight_new = r'''            m_undoStack->push(new AddAnnotationCommand(m_scene, newHighlightItem));'''
content = content.replace(highlight_old, highlight_new)

underline_old = r'''            m_scene->addItem(underline);'''
underline_new = r'''            m_undoStack->push(new AddAnnotationCommand(m_scene, underline));'''
content = content.replace(underline_old, underline_new)

with open(pdfview_cpp_path, 'w', encoding='utf-8') as f:
    f.write(content)

mainwindow_cpp_path = r'e:\Programing\AcrobatKiller\src\MainWindow.cpp'
with open(mainwindow_cpp_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Add undo/redo shortcuts and toolbar actions
content = content.replace('#include <QMenuBar>', '#include <QMenuBar>\n#include <QUndoStack>')

undo_redo_actions = r'''    QAction* saveAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "&Save Annotations");
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(saveAction, &QAction::triggered, this, [this]() { m_pdfView->saveAnnotations(m_currentFilePath); });

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QAction* undoAction = m_pdfView->undoStack()->createUndoAction(this, "&Undo");
    undoAction->setShortcut(QKeySequence("Ctrl+Z"));
    undoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    editMenu->addAction(undoAction);

    QAction* redoAction = m_pdfView->undoStack()->createRedoAction(this, "&Redo");
    redoAction->setShortcut(QKeySequence("Ctrl+Y"));
    redoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    editMenu->addAction(redoAction);
'''

content = content.replace(r'''    QAction* saveAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "&Save Annotations");
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(saveAction, &QAction::triggered, this, [this]() { m_pdfView->saveAnnotations(m_currentFilePath); });''', undo_redo_actions)

toolbar_old = r'''    m_fileToolBar->addAction(openAction);
    m_fileToolBar->addAction(saveAction);
    m_fileToolBar->addSeparator();'''
toolbar_new = r'''    m_fileToolBar->addAction(openAction);
    m_fileToolBar->addAction(saveAction);
    m_fileToolBar->addSeparator();
    m_fileToolBar->addAction(undoAction);
    m_fileToolBar->addAction(redoAction);
    m_fileToolBar->addSeparator();'''
content = content.replace(toolbar_old, toolbar_new)

with open(mainwindow_cpp_path, 'w', encoding='utf-8') as f:
    f.write(content)

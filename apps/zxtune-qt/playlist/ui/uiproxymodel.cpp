/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "uiproxymodel.h"

#ifndef QT_NO_PROXYMODEL
#include "uiproxymodel_p.h"
#include <QtCore/QSize.h>
#include <QtCore/QStringList.h>

QT_BEGIN_NAMESPACE_UIHELPERS

/*!
    \class UIProxyModel
    \obsolete
    \brief The UIProxyModel class provides support for processing data
    passed between another model and a view.

    \ingroup model-view
    \inmodule QtWidgets

    If you want to do filtering and sorting, see QSortFilterProxyModel.

    Proxy models provide a standard model interface that can be used to
    manipulate the data retrieved through an underlying model. They can be used to
    perform operations such as sorting and filtering on the data obtained without
    changing the contents of the model.

    Just as with subclasses of QAbstractItemView, UIProxyModel provides the setModel()
    function that is used to specify the model to be acted on by the proxy.
    Views can be connected to either the underlying model or the proxy model with
    \l QAbstractItemView::setModel().

    Since views rely on the information provided in model indexes to identify
    items of data from models, and to position these items in some visual
    representation, proxy models must create their own model indexes instead of
    supplying model indexes from their underlying models.

    \sa {Model/View Programming}, QAbstractItemModel

*/

/*!
    Constructs a proxy model with the given \a parent.
*/

UIProxyModel::UIProxyModel(QObject *parent)
    : QAbstractItemModel(*new UIProxyModelPrivate, parent)
{
    Q_D(UIProxyModel);
    setModel(&d->empty);
}

/*!
    \internal
*/
UIProxyModel::UIProxyModel(UIProxyModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(UIProxyModel);
    setModel(&d->empty);
}

/*!
    Destroys the proxy model.
*/
UIProxyModel::~UIProxyModel()
{
}

/*!
    Sets the given \a model to be processed by the proxy model.
*/
void UIProxyModel::setModel(QAbstractItemModel *model)
{
    Q_D(UIProxyModel);
    if (d->model && d->model != &d->empty)
        disconnectFromModel(d->model);
    if (model) {
        d->model = model;
        connectToModel(model);
    } else {
        d->model = &d->empty;
    }
}

/*!
    Returns the model that contains the data that is available through the
    proxy model.
*/
QAbstractItemModel *UIProxyModel::model() const
{
    Q_D(const UIProxyModel);
    return d->model;
}

/*!
    Returns the model index with the given \a row, \a column, and \a parent.

    \sa QAbstractItemModel::index()
*/
QModelIndex UIProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const UIProxyModel);
    return setProxyModel(d->model->index(row, column, setSourceModel(parent)));
}

/*!
    Returns the model index that corresponds to the parent of the given \a child
    index.
*/
QModelIndex UIProxyModel::parent(const QModelIndex &child) const
{
    Q_D(const UIProxyModel);
    return setProxyModel(d->model->parent(setSourceModel(child)));
}

/*!
    Returns the number of rows for the given \a parent.

    \sa QAbstractItemModel::rowCount()
*/
int UIProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const UIProxyModel);
    return d->model->rowCount(setSourceModel(parent));
}

/*!
    Returns the number of columns for the given \a parent.

    \sa QAbstractItemModel::columnCount()
*/
int UIProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const UIProxyModel);
    return d->model->columnCount(setSourceModel(parent));
}

/*!
    Returns true if the item corresponding to the \a parent index has child
    items; otherwise returns false.

    \sa QAbstractItemModel::hasChildren()
*/
bool UIProxyModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const UIProxyModel);
    return d->model->hasChildren(setSourceModel(parent));
}

/*!
    Returns the data stored in the item with the given \a index under the
    specified \a role.
*/
QVariant UIProxyModel::data(const QModelIndex &index, int role) const
{
    Q_D(const UIProxyModel);
    return d->model->data(setSourceModel(index), role);
}

/*!
    Sets the \a role data for the item at \a index to \a value.
    Returns true if successful; otherwise returns false.

    The base class implementation returns false. This function and
    data() must be reimplemented for editable models.

    \sa data(), itemData(), QAbstractItemModel::setData()
*/
bool UIProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(const UIProxyModel);
    return d->model->setData(setSourceModel(index), value, role);
}

/*!
    Returns the data stored in the \a section of the header with specified
    \a orientation under the given \a role.
*/
QVariant UIProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const UIProxyModel);
    return d->model->headerData(section, orientation, role);
}

/*!
    Sets the \a role data in the \a section of the header with the specified
    \a orientation to the \a value given.

    \sa QAbstractItemModel::setHeaderData()
*/
bool UIProxyModel::setHeaderData(int section, Qt::Orientation orientation,
                                const QVariant &value, int role)
{
    Q_D(const UIProxyModel);
    return d->model->setHeaderData(section, orientation, value, role);
}

/*!
    Returns a list of MIME types that are supported by the model.
*/
QStringList UIProxyModel::mimeTypes() const
{
    Q_D(const UIProxyModel);
    return d->model->mimeTypes();
}

/*!
    Returns MIME data for the specified \a indexes in the model.
*/
QMimeData *UIProxyModel::mimeData(const QModelIndexList &indexes) const
{
    Q_D(const UIProxyModel);
    QModelIndexList lst;
    for (int i = 0; i < indexes.count(); ++i)
        lst.append(setSourceModel(indexes.at(i)));
    return d->model->mimeData(lst);
}

/*!
    Returns true if the model accepts the \a data dropped onto an attached
    view for the specified \a action; otherwise returns false.

    The \a parent, \a row, and \a column details can be used to control
    which MIME types are acceptable to different parts of a model when
    received via the drag and drop system.
*/
bool UIProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                               int row, int column, const QModelIndex &parent)
{
    Q_D(const UIProxyModel);
    return d->model->dropMimeData(data, action, row, column, setSourceModel(parent));
}

/*!
    Returns the drop actions that are supported by the model; this is
    a combination of the individual actions defined in \l Qt::DropActions.

    The selection of drop actions provided by the model will influence the
    behavior of the component that started the drag and drop operation.

    \sa \l{dnd.html}{Drag and Drop}
*/
Qt::DropActions UIProxyModel::supportedDropActions() const
{
    Q_D(const UIProxyModel);
    return d->model->supportedDropActions();
}

/*!
    Inserts \a count rows into the model, creating new items as children of
    the given \a parent. The new rows are inserted before the \a row
    specified. If the \a parent item has no children, a single column is
    created to contain the required number of rows.

    Returns true if the rows were successfully inserted; otherwise
    returns false.

    \sa QAbstractItemModel::insertRows()
*/
bool UIProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_D(const UIProxyModel);
    return d->model->insertRows(row, count, setSourceModel(parent));
}

/*!
    Inserts \a count columns into the model, creating new items as children of
    the given \a parent. The new columns are inserted before the \a column
    specified. If the \a parent item has no children, a single row is created
    to contain the required number of columns.

    Returns true if the columns were successfully inserted; otherwise
    returns false.

    \sa QAbstractItemModel::insertColumns()
*/
bool UIProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(const UIProxyModel);
    return d->model->insertColumns(column, count, setSourceModel(parent));
}

/*!
    Fetches more child items of the given \a parent. This function is used by views
    to tell the model that they can display more data than the model has provided.

    \sa QAbstractItemModel::fetchMore()
*/
void UIProxyModel::fetchMore(const QModelIndex &parent)
{
    Q_D(const UIProxyModel);
    d->model->fetchMore(parent);
}

/*!
    Returns the item flags for the given \a index.
*/
Qt::ItemFlags UIProxyModel::flags(const QModelIndex &index) const
{
    Q_D(const UIProxyModel);
    return d->model->flags(setSourceModel(index));
}

/*!
    Sorts the child items in the specified \a column according to the sort
    order defined by \a order.

    \sa QAbstractItemModel::sort()
*/
void UIProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_D(UIProxyModel);
    d->model->sort(column, order);
}

/*!
    Returns a list of model indexes that each contain the given \a value for
    the \a role specified. The search begins at the \a start index and is
    performed according to the specified \a flags. The search continues until
    the number of matching data items equals \a hits, the last row is reached,
    or the search reaches \a start again, depending on whether \c MatchWrap is
    specified in \a flags.

    \sa QAbstractItemModel::match()
*/
QModelIndexList UIProxyModel::match(const QModelIndex &start,
                                   int role, const QVariant &value,
                                   int hits, Qt::MatchFlags flags) const
{
    Q_D(const UIProxyModel);
    return d->model->match(start, role, value, hits, flags);
}

/*!
    Returns the size of the item that corresponds to the specified \a index.
*/
QSize UIProxyModel::span(const QModelIndex &index) const
{
    Q_D(const UIProxyModel);
    return d->model->span(setSourceModel(index));
}

/*!
 */
bool UIProxyModel::submit()
{
    Q_D(UIProxyModel);
    return d->model->submit();
}

/*!
 */
void UIProxyModel::revert()
{
    Q_D(UIProxyModel);
    d->model->revert();
}

struct HackModelIndex
{
    HackModelIndex(int _r, int _c, void *_p, const QAbstractItemModel *_m)
      : r(_r), c(_c), p(_p), m(_m)
    {

    }

    operator QModelIndex() { return reinterpret_cast<QModelIndex&>(*this); }

    int r, c;
    void *p;
    const QAbstractItemModel *m;
};

/*!
    \internal
    Change the model pointer in the given \a source_index to point to the proxy model.
 */
QModelIndex UIProxyModel::setProxyModel(const QModelIndex &source_index) const
{
    return HackModelIndex(source_index.row(), source_index.column(), source_index.internalPointer(), this);
}

/*!
    \internal
    Change the model pointer in the given \a proxy_index to point to the source model.
 */
QModelIndex UIProxyModel::setSourceModel(const QModelIndex &proxy_index) const
{
    Q_D(const UIProxyModel);
    return HackModelIndex(proxy_index.row(), proxy_index.column(), proxy_index.internalPointer(), d->model);
}

/*!
  \internal
  Connect to all the signals emitted by given \a model.
*/
void UIProxyModel::connectToModel(const QAbstractItemModel *model) const
{
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(_q_sourceDataChanged(QModelIndex,QModelIndex)));
    connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SIGNAL(headerDataChanged(Qt::Orientation,int,int))); // signal to signal
    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceRowsRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this, SLOT(_q_sourceColumnsRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(modelReset()), this, SIGNAL(modelReset())); // signal to signal
    connect(model, SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged())); // signal to signal
    connect(model, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged())); // signal to signal
}

/*!
  \internal
  Disconnect from all the signals emitted by the given \a model.
 */
void UIProxyModel::disconnectFromModel(const QAbstractItemModel *model) const
{
    disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this, SLOT(_q_sourceDataChanged(QModelIndex,QModelIndex)));
    disconnect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
               this, SIGNAL(headerDataChanged(Qt::Orientation,int,int))); // signal to signal
    disconnect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
               this, SLOT(_q_sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(rowsInserted(QModelIndex,int,int)));
    disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
               this, SLOT(_q_sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
               this, SLOT(_q_sourceRowsRemoved(QModelIndex,int,int)));
    disconnect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
               this, SLOT(_q_sourceColumnsAboutToBeInserted(QModelIndex,int,int)));
    disconnect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
               this, SLOT(_q_sourceColumnsInserted(QModelIndex,int,int)));
    disconnect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
               this, SLOT(_q_sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));
    disconnect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
               this, SLOT(_q_sourceColumnsRemoved(QModelIndex,int,int)));
    disconnect(model, SIGNAL(modelReset()), this, SIGNAL(modelReset())); // signal to signal
    disconnect(model, SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged())); // signal to signal
    disconnect(model, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged())); // signal to signal
}

/*!
  \fn QObject *UIProxyModel::parent() const
  \internal
*/

void UIProxyModelPrivate::_q_sourceDataChanged(const QModelIndex &tl,const QModelIndex &br)
{
    Q_Q(UIProxyModel);
    emit q->dataChanged(q->setProxyModel(tl), q->setProxyModel(br));
}

void UIProxyModelPrivate::_q_sourceRowsAboutToBeInserted(const QModelIndex &parent, int first ,int last)
{
    Q_Q(UIProxyModel);
    q->beginInsertRows(q->setProxyModel(parent), first, last);
}

void UIProxyModelPrivate::_q_sourceRowsInserted(const QModelIndex &, int, int)
{
    Q_Q(UIProxyModel);
    q->endInsertRows();
}

void UIProxyModelPrivate::_q_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_Q(UIProxyModel);
    q->beginRemoveRows(q->setProxyModel(parent), first, last);
}

void UIProxyModelPrivate::_q_sourceRowsRemoved(const QModelIndex &, int, int)
{
    Q_Q(UIProxyModel);
    q->endRemoveRows();
}

void UIProxyModelPrivate::_q_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    Q_Q(UIProxyModel);
    q->beginInsertColumns(q->setProxyModel(parent), first, last);
}

void UIProxyModelPrivate::_q_sourceColumnsInserted(const QModelIndex &, int, int)
{
    Q_Q(UIProxyModel);
    q->endInsertColumns();
}

void UIProxyModelPrivate::_q_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_Q(UIProxyModel);
    q->beginRemoveColumns(q->setProxyModel(parent), first, last);
}


void UIProxyModelPrivate::_q_sourceColumnsRemoved(const QModelIndex &, int, int)
{
    Q_Q(UIProxyModel);
    q->endRemoveColumns();
}

QT_END_NAMESPACE_UIHELPERS

//#include "uiproxymodel_p.moc.cpp"

#endif // QT_NO_PROXYMODEL

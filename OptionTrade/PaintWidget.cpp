#include "stdafx.h"
#include <QtGui>
#include <QtWidgets>
#include <QMainWindow>
#include <QJsonArray>
#include "PaintWidget.h"
#include "MainWindow.h"

PaintWidget::PaintWidget(QWidget *parent, ImageProcessing *p) :
	QLabel(parent)
{
	I_MainWindow = (ImageProcessing*)p;
	isDrawing = false;

	image_file_name = "";
	m_nScaleInx = 0;
	
	m_bZoom = false;
	m_bleftZoom = false;
	m_bDrawZoomImage = false;
	m_bFivePoint = false;
	m_bDrag = false;
	//m_bDrawAverRectDone = false;
	m_qpOffsetTotal = QPoint(0, 0);
	//m_tMousePoint = QPoint(width() / 2, height() / 2);
	m_tLastCursorShape = Qt::ArrowCursor;

	//-----标定相关------
	setMouseTracking(true);
	draw_mode = MODE_NONE;
	draw_type = shapeData::eShapeType::ploygon;
	
	bBtnOk = false;
	bBtnCancel = false;
	move_shape = false;
	selected_vertex_index = -1;

	connect(this, &PaintWidget::Signal_Zoom, this, &PaintWidget::ZoomRequest);
}

void PaintWidget::setImageName(QString image_name)
{
	image_file_name = image_name;
}

void PaintWidget::setImage(QImage img)				// 读入图像
{
	image = img; 
	tempImage = image;
}

QImage PaintWidget::getImage()						// 外部获取图像
{
	if (image.isNull() != true)
		return image;
}

void PaintWidget::setShape(PaintWidget::shape t)	// 设置绘制类型
{
	type = t;
}

void PaintWidget::setPenWidth(int w)					// 设置画笔宽度
{
	penWidth = w;
}

void PaintWidget::setPenColor(QColor c)							// 设置画笔颜色
{
	penColor = c;
}

void PaintWidget::paintEvent(QPaintEvent * pEvent)
{
	QLabel::paintEvent(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	
	if (isDrawing == true) 
	{
		//painter.drawImage(0, 0, tempImage);// 如果正在绘图，既鼠标点击或者正在移动，画在tempImage上  
	}
	
	//if(m_bMousePress)
	//{
		//painter.drawImage(0, 0, image);// 如果鼠标释放，将图保存在image上

		QRect target(2, 2, width() - 4, height() - 4);
		QRect source(1, 1, image.width(), image.height());
		float fScale = m_fScaleList[m_nScaleInx];
		int nScaledW = image.width() / fScale;
		int nScaledH = image.height() / fScale;
		int nMouseOffsetX = 0;
		int nMouseOffsetY = 0;
		//移动
		if (action == Move)
		{
			int offsetx = m_qpOffsetTotal.x() + m_qpCurrentOffset.x();
			m_qpOffsetTotal.setX(offsetx);

			int offsety = m_qpOffsetTotal.y() + m_qpCurrentOffset.y();
			m_qpOffsetTotal.setY(offsety);
			action = None;

		}
#if true
		int nWidth = width();
		int nHeight = height();
		if (abs(m_qpOffsetTotal.x()) >= (nWidth / 4))    //限制X偏移值
		{
			if (m_qpOffsetTotal.x() > 0)
				m_qpOffsetTotal.setX(nWidth / 4);
			else
				m_qpOffsetTotal.setX(-nWidth / 4);
		}
		if (abs(m_qpOffsetTotal.y()) >= (height() / 4))    //限制Y偏移值
		{
			if (m_qpOffsetTotal.y() > 0)
				m_qpOffsetTotal.setY(height() / 4);
			else
				m_qpOffsetTotal.setY(-height() / 4);

		}
#endif
		//qDebug("offset:%d\n", m_qpOffsetTotal);
		int  sx = image.width() / 2 - width() / 2 - m_qpOffsetTotal.x();
		int  sy = image.height() / 2 - height() / 2 - m_qpOffsetTotal.y();

		int nOrignX = image.width() / 2 - (image.width() / fScale / 2) + nMouseOffsetX;
		int nOrignY = image.height() / 2 - (image.height() / fScale / 2) + nMouseOffsetY;
		if (m_bDrag)
		{
			nOrignX += sx;
			nOrignY += sy;
		}
		source.setLeft(nOrignX);
		source.setTop(nOrignY);;
		source.setWidth(image.width() / fScale == 0 ? 1 : image.width() / fScale);
		source.setHeight(image.height() / fScale == 0 ? 1 : image.height() / fScale);
		//if (m_nScaleInx > 0)
		//{
		//	painter.drawImage(target, image, source);
		//}
		//else
		//{
		//	painter.drawImage(0, 0, image);// 如果鼠标释放，将图保存在image上
		//}
		resize(image.width() * fScale, image.height() * fScale);
	//}

	//-----标定相关------
	//画多个shape
	std::for_each(all_shapes.begin(), all_shapes.end(), [=](shapeData& s)
	{
		if (s.IsVisibel())
			paintShape(s, QColor(Qt::green), QColor(Qt::green), fScale);

	});

	paintShape(current_shape, QColor(Qt::green), QColor(Qt::green), fScale);

	paintShape(line, QColor(Qt::green), QColor(Qt::green), fScale);

	//if (!image.isNull())
	//{
		//QPixmap pixmap;
		//pixmap.convertFromImage(image);
		//painter.drawPixmap(0, 0, pixmap);
	//}
}

void PaintWidget::mousePressEvent(QMouseEvent *event) 
{
	if (event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos();
		lastPoint = pos;
		isDrawing = true;// 鼠标点击开始绘图，移动表示正在绘图
		m_bMousePress = true;

		if (IsDrawing())
		{
			I_MainWindow->setActionEditPolygonEnbaled(false);
			current_shape.setShapeType(draw_type);

			if (outOfPixmap(pos))
			{
				qDebug() << "mousePressEvent IsDrawing current pos is out of image" << endl;
				return;
			}

			float fScale = m_fScaleList[m_nScaleInx];
			current_shape.addPoint(pos/ fScale);
			update();
		}
		else
		{
			selectShapePoint(pos);
			prev_point = pos;
		}
	}
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event) 
{
	if (event->buttons() & Qt::LeftButton)
	{// 鼠标为左键且正在移动  
		endPoint = event->pos();
		tempImage = image;
		if (type == Pen)
		{// 如果工具为画笔，不用双缓冲直接画在画板上  
			//paint(image);
		}
		else
		{ // 否则用双缓冲绘图  
			//paint(tempImage);
		}

		if (m_bMousePress && !move_shape && !IsDrawing())
		{
			QMouseEvent *mouse = dynamic_cast<QMouseEvent*>(event);
			m_qpCurrentOffset.setX(mouse->x() - lastPoint.x());
			m_qpCurrentOffset.setY(mouse->y() - lastPoint.y());
			action = Move;
		}
	}

	QPoint pos = event->pos();
	float fScale = m_fScaleList[m_nScaleInx];

	if (IsDrawing())
	{
		shapeData::PointsType points = current_shape.getPoints();
		shapeData::PointsType temp_points;

		if (outOfPixmap(pos))
		{
			qDebug() << "mouseMoveEvent IsDrawing current pos is out of image" << endl;
			return;
		}

		if (current_shape.IsRectangle() && points.size() >= 1)
		{
			temp_points.push_back(points[0]);
			temp_points.push_back(pos / fScale);
			current_shape.setPoints(temp_points);
			qDebug("mouseMoveEvent IsDrawing IsRectangle current pos ：(%d,%d)",temp_points[1].x(), temp_points[1].y());
			repaint();
		}

		//画多边形时，提示下一步标定轮廓
		if (current_shape.IsPloygon() && points.size() >= 1/* && bBtnCancel*/)
		{
			line.setShapeType(shapeData::eShapeType::ploygon);
			temp_points.push_back(points[points.size() - 1]);
			temp_points.push_back(pos / fScale);
			line.setPoints(temp_points);
			repaint();

		}
	}

	if (IsEditing())
	{
		setCursor(Qt::ArrowCursor);

		// Polygon / Vertex moving.
		if (event->buttons() & Qt::LeftButton)
		{
			if (selected_vertex_index >= 0)//选中顶点
			{
				boundedMoveShapes(h_shape, pos);
				updateShape(h_shape);
				move_shape = true;
				repaint();
			}
			else if (selected_shape.getIndex() >= 0 && selected_shape.IsSelected())
			{
				setCursor(Qt::ClosedHandCursor);
				boundedMoveShapes(selected_shape, pos);
				updateShape(selected_shape);
				move_shape = true;
				repaint();
			}
			else
			{
				move_shape = false;
			}
			return;
		}

		// Just hovering over the canvas, 2 posibilities:
		// - Highlight shapes
		// - Highlight vertex
		// Update shape/vertex fill and tooltip value accordingly.
		std::for_each(all_shapes.begin(), all_shapes.end(), [=](shapeData& s)
		{
			if (s.IsVisibel())
			{
				qreal epsilon = 10.0;
				int point_index = -1;
				bool bSelectedVertex = nearestVertex(s, pos, epsilon, point_index);
				if (bSelectedVertex)
				{
					h_shape = s;
					selected_vertex_index = point_index;
					
					s.setFill(true);
					s.setHighlightPointIndex(point_index);
					setToolTip(QString("Click & drag to move point"));
					setStatusTip(toolTip());
					setCursor(Qt::PointingHandCursor);
					repaint();
				}
				else if (containsPoint(s, pos))
				{
					selected_vertex_index = point_index;
					
					s.setFill(true);
					setToolTip(QString("Click & drag to move shape '%1'").arg(s.getLabel()));
					setStatusTip(toolTip());
					setCursor(Qt::OpenHandCursor);
					repaint();
				}
				else
				{
					s.setFill(false);

					//highlightClear
					s.setHighlightPointIndex(-1);
					repaint();
				}
			}
		});
	}
}

void PaintWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDrawing = false;
		m_bMousePress = false;
		if (type != Pen)
		{
			paint(image);
		}

		if (IsDrawing() && current_shape.IsRectangle() && current_shape.getPoints().size() >= 2)//多边形至少需要2个点
		{
			emit newShape(current_shape);
			finalise();

		}

		//move_shape为true，表示移动顶点；鼠标释放表示移动完成，故对selected_vertex_index做clear处理（赋值-1）
		if (move_shape)
		{
			selected_vertex_index = -1;
		}
	}
}

void PaintWidget::mouseDoubleClickEvent(QMouseEvent*event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (IsDrawing() && current_shape.IsPloygon() && current_shape.getPoints().size() >= 3)  //多边形至少需要3个点
		{
			//current_shape.setPloygonShutting(true);
			//通过画线，实现多边形自动闭合
			shapeData::PointsType points = current_shape.getPoints();
			shapeData::PointsType temp_points;
			line.setShapeType(shapeData::eShapeType::ploygon);
			temp_points.push_back(points[points.size() - 1]);
			temp_points.push_back(points[0]);
			line.setPoints(temp_points);
			repaint();

			emit newShape(current_shape);
			finalise();
		}
	}
}

void PaintWidget::enterEvent(QEvent *event)
{
	if (IsDrawing())
	{
		setCursor(QCursor(Qt::CrossCursor));
	}
}

void PaintWidget::leaveEvent(QEvent *event)
{
	QApplication::changeOverrideCursor(QCursor(Qt::ArrowCursor));
}

void PaintWidget::paint(QImage &theImage) {
	QPainter p(&theImage);
	QPen apen;	
	apen.setWidth(penWidth);	// 设置画笔宽度
	apen.setColor(penColor);	// 设置画笔颜色
	p.setPen(apen);// 设置绘图工具画笔线条宽度为4  				  
	p.setRenderHint(QPainter::Antialiasing, true); // 反走样

	int x1, y1, x2, y2;
	x1 = lastPoint.x();
	y1 = lastPoint.y();
	x2 = endPoint.x();
	y2 = endPoint.y();

	switch (type) {					// 画图 
	case PaintWidget::Pen: 
	{
		p.drawLine(lastPoint, endPoint);
		lastPoint = endPoint;
		break;
	}
	case  PaintWidget::Line: 
	{
		p.drawLine(lastPoint, endPoint);
		break;
	}
	case PaintWidget::Ellipse: 
	{
		p.drawEllipse(x1, y1, x2 - x1, y2 - y1);
		break;
	}
	case PaintWidget::Circle: 
	{
		double length = (x2 - x1) > (y2 - y1) ? (x2 - x1) : (y2 - y1);
		p.drawEllipse(x1, y1, length, length);
		break;
	}
	case PaintWidget::Triangle: 
	{
		int top, buttom, left, right;
		top = (y1 < y2) ? y1 : y2;
		buttom = (y1 > y2) ? y1 : y2;
		left = (x1 < x2) ? x1 : x2;
		right = (x1 > x2) ? x1 : x2;

		if (y1 < y2)
		{
			QPoint points[3] = { QPoint(left,buttom),	QPoint(right,buttom),	QPoint((right + left) / 2,top) };
			p.drawPolygon(points, 3);
		}
		else
		{
			QPoint points[3] = { QPoint(left,top),	QPoint(right,top),	QPoint((left + right) / 2,buttom) };
			p.drawPolygon(points, 3);
		}
		break;
	}
	case PaintWidget::Rhombus: 
	{
		int top, buttom, left, right;
		top = (y1 < y2) ? y1 : y2;
		buttom = (y1 > y2) ? y1 : y2;
		left = (x1 < x2) ? x1 : x2;
		right = (x1 > x2) ? x1 : x2;

		QPoint points[4] = { 
			QPoint(left,(top + buttom) / 2),
			QPoint((left + right) / 2,buttom),
			QPoint(right,(top + buttom) / 2), 
			QPoint((left + right) / 2,top) };
		p.drawPolygon(points, 4);
		break;
	}
	case PaintWidget::Rect: 
	{
		p.drawRect(x1, y1, x2 - x1, y2 - y1);
		break;
	}
	case PaintWidget::Square: 
	{
		double length = (x2 - x1) > (y2 - y1) ? (x2 - x1) : (y2 - y1);
		p.drawRect(x1, y1, length, length);
		break;
	}
	case PaintWidget::Hexagon: 
	{
		QPoint points[6] = {
			QPoint(x1,y1),
			QPoint(x2,y1),
			QPoint((3 * x2 - x1) / 2,(y1 + y2) / 2),
			QPoint(x2,y2),
			QPoint(x1,y2),
			QPoint((3 * x1 - x2) / 2,(y1 + y2) / 2) };
		p.drawPolygon(points, 6);
		break;
	}
	case PaintWidget::Null:
	{
		;
	}
	default:
		break;
	}

	update();// 重绘  
}

void PaintWidget::wheelEvent(QWheelEvent *event)
{
	int wheel_val = event->delta();
	//double scaleRatio = ((double)wheel_val) / RATIO_TO_DOULBE;

	if (m_nScaleInx == 0)
	{
		m_qpOffsetTotal.setX(image.width() / 2 - width() / 2);
		m_qpOffsetTotal.setY(image.height() / 2 - height() / 2);
	}

	if (event->delta() > 0)//"上滚"
	{
		m_nScaleInx++;
		if (m_nScaleInx > 13)
		{
			m_nScaleInx = 13;
		}

		action = Amplification;
	}
	else//"下滚"
	{
		m_nScaleInx--;
		if (m_nScaleInx < 0)
		{
			m_nScaleInx = 0;
		}
		action = Shrink;
	}
	float fScale = m_fScaleList[m_nScaleInx];
	if (fScale > 1)
	{
		m_tLastCursorShape = m_bZoomCenter ? Qt::CrossCursor : Qt::OpenHandCursor;
		//QApplication::setOverrideCursor(m_tLastCursorShape); //设置鼠标样式
		setCursor(m_tLastCursorShape);
		m_bDrag =  true;
	}
	else
	{
		//QApplication::setOverrideCursor(Qt::ArrowCursor);  //设置鼠标样式
		setCursor(Qt::ArrowCursor);
		m_tLastCursorShape = Qt::ArrowCursor;
		m_bDrag = false;
	}
	emit Signal_Zoom(m_fScaleList[m_nScaleInx]);


	update();

	int nX = event->x();
	int nY = event->y();
	int nGlobalX = event->globalX();
	int nGlobalY = event->globalY();

	event->accept();
}

void PaintWidget::contextMenuEvent(QContextMenuEvent *ev)
{
	if (!I_MainWindow)
	{
		return;
	}

	QMenu *menu = new QMenu();
	menu->addSeparator();
	menu->addSeparator();

	//添加context menu
	menu->addAction(I_MainWindow->get_act_classification());
	menu->addAction(I_MainWindow->get_act_draw_rectangle());
	menu->addAction(I_MainWindow->get_act_draw_ploygon());
	menu->addAction(I_MainWindow->get_act_edit_ploygon());
	menu->addAction(I_MainWindow->get_act_delete_ploygon());
	menu->addAction(I_MainWindow->get_act_edit_label());
	menu->addAction(I_MainWindow->get_act_save_label());

	menu->addSeparator();
	menu->addSeparator();
	menu->addSeparator();
	menu->exec(ev->globalPos());
	delete menu;
}

/******************************************************************************
* 函数名:slot_action_drag
* 描述: 鼠标拖动图片
* IN  :
* OUT:
* 返回值:
******************************************************************************/
void PaintWidget::slot_action_DrawShape()
{
	isDrawing = !isDrawing;
}

//-------标定相关----------------
void PaintWidget::paintShape(shapeData shape_object, QColor line_color, QColor fill_color, float fScale)
{
	QPainter painter(this);

	QPainterPath line_path;
	QPainterPath vertex_path;// 画各个点

	vertex_path.setFillRule(Qt::WindingFill); //设置顶点的填充方式
	painter.setPen(QPen(line_color));         //设置线的颜色

	shapeData::PointsType points = shape_object.getPoints();

	if (shape_object.IsRectangle())
	{
		if (points.size() >= 2)
		{
			QPoint p1 = points[0] * fScale;
			QPoint p2 = points[1] * fScale;
			qreal x2 = p2.x();
			qreal y2 = p2.y();
			qreal x1 = p1.x();
			qreal y1 = p1.y();
			QRectF tf(x1, y1, x2 - x1, y2 - y1);

			line_path.addRect(tf);

			shape_object.drawVertex(vertex_path, 0, fScale);
			shape_object.drawVertex(vertex_path, 1, fScale);
		}
	}

	if (shape_object.IsPloygon())
	{
		if (points.size() > 0)
		{
			line_path.moveTo(points[0] * fScale);
			for (int i = 0; i < points.size(); ++i)
			{
				line_path.lineTo(points[i] * fScale);
				shape_object.drawVertex(vertex_path, i, fScale);
			}

			if (shape_object.IsPloygonShutting())
			{
				line_path.lineTo(points[0] * fScale); //这里是保证多边形是闭合
			}

		}
	}

	painter.drawPath(line_path);

	painter.drawPath(vertex_path);
	painter.fillPath(vertex_path, QBrush(shape_object.getVertexFillColor()));

	if (shape_object.IsFill() || shape_object.IsSelected())
	{
		fill_color = shape_object.IsSelected() ? QColor(0, 128, 255, 155) : QColor(255, 0, 0, 128);
		painter.fillPath(line_path, fill_color);
	}
	else
	{
		painter.fillPath(line_path, Qt::transparent);
	}

}

void PaintWidget::finalise()
{
	if (bBtnOk)
	{
		addShape(current_shape);
		//all_shapes.push_back(current_shape);
		current_shape.clearData();
		line.clearData();
		update();
	}
}

int PaintWidget::getMaxIndex()
{
	int ret = -1;

	ShapeDataType::iterator iter = max_element(all_shapes.begin(), all_shapes.end(), shape_comp());
	if (iter != all_shapes.end())
	{
		ret = iter->getIndex();
	}

	return ret;
}

bool PaintWidget::get_shape_from_item(int index, shapeData & shape_unit)
{
	bool ret = false;
	ShapeDataType::iterator iter = std::find_if(all_shapes.begin(), all_shapes.end(), [=](shapeData& s) {return s.getIndex() == index; });
	if (iter != all_shapes.end())
	{
		shape_unit = *iter;
		ret = true;
	}
	return ret;
}

void PaintWidget::addShape(const shapeData& shape_unit)
{
	all_shapes.push_back(shape_unit);
}

void PaintWidget::updateShape(const shapeData& shape_unit)
{
	ShapeDataType::iterator iter = std::find_if(all_shapes.begin(), all_shapes.end(), [=](shapeData& s) {return s.getIndex() == shape_unit.getIndex(); });
	if (iter != all_shapes.end())
	{
		*iter = shape_unit;
		repaint();
	}
}

void PaintWidget::updateLabel(int index, QString label)
{
	ShapeDataType::iterator iter = std::find_if(all_shapes.begin(), all_shapes.end(), [=](shapeData& s) {return s.getIndex() == index; });
	if (iter != all_shapes.end())
	{
		iter->setLabel(label);
	}
}

void PaintWidget::clearAllSelectShape()
{
	std::for_each(all_shapes.begin(), all_shapes.end(), [=](shapeData& s)
	{
		s.setSelected(false);
		s.setFill(false);
	});
	repaint();
}

void PaintWidget::clearSelectShape()
{
	//std::for_each(all_shapes.begin(), all_shapes.end(), [=](shapeData& s)
	//{
	//	s.setNotSelected(true);
	//});
	selected_shape.setSelected(false);
	updateShape(selected_shape);
}

void PaintWidget::clearAllShapeData()
{
	all_shapes.clear();
}

void PaintWidget::removeShape(int index)
{
	ShapeDataType::iterator iter = all_shapes.begin();
	while (iter != all_shapes.end())
	{
		if (iter->getIndex() == index)
		{
			iter = all_shapes.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
	repaint();
}

bool PaintWidget::getCurrentSelectedShape(shapeData & shape_unit)
{
	bool ret = false;
	ShapeDataType::iterator iter = std::find_if(all_shapes.begin(), all_shapes.end(), [=](shapeData& s) {return s.IsSelected(); });
	if (iter != all_shapes.end())
	{
		shape_unit = *iter;
		ret = true;
	}
	return ret;
}

QPainterPath* PaintWidget::makePath(shapeData shape_unit)
{
	QPainterPath *path = nullptr;
	shapeData::PointsType points = shape_unit.getPoints();
	convertToViewPoints(points);
	if (shape_unit.IsRectangle() && points.size() >= 2)
	{
		QRectF rect = getRectFromLine(points[0], points[1]);
		path = new QPainterPath;
		path->addRect(rect);

	}

	if (shape_unit.IsPloygon() && points.size() >= 1)
	{
		path = new QPainterPath(points[0]);
		path->moveTo(points[0]);
		for (int i = 1; i < points.size(); ++i)
			path->lineTo(points[i]);
		return path;
	}
	return path;
}

bool PaintWidget::containsPoint(shapeData shape_unit, QPoint point)
{
	bool ret = false;
	QPainterPath *path = makePath(shape_unit);
	if (path)
	{
		ret = path->contains(point);
		delete path;
	}

	return ret;
}

QRectF PaintWidget::getRectFromLine(QPoint p1, QPoint p2)
{
	qreal x2 = p2.x();
	qreal y2 = p2.y();
	qreal x1 = p1.x();
	qreal y1 = p1.y();
	QRectF tf(x1, y1, x2 - x1, y2 - y1);
	return tf;
}

bool PaintWidget::nearestVertex(shapeData shape_unit, QPoint point, qreal epsilon, int& point_index)
{
	bool ret = false;
	qreal min_distance = DBL_MAX;
	shapeData::PointsType points = shape_unit.getPoints();
	convertToViewPoints(points);

	qreal dist = DBL_MAX;
	for (int i = 0; i < points.size(); ++i)
	{
		qreal dist = Distance(points[i], point);
		if (dist <= epsilon && dist < min_distance)
		{
			min_distance = dist;
			point_index = i;
			ret = true;
		}
	}

	return ret;
}

qreal PaintWidget::Distance(QPoint p1, QPoint p2)
{
	qreal x = p1.x() - p2.x();
	qreal y = p1.y() - p2.y();
	qreal dist = sqrt(x*x + y * y);
	return dist;
}

void PaintWidget::boundedMoveShapes(shapeData &shape_unit, QPoint pos)
{
	if (outOfPixmap(pos))
	{
		qDebug() << "mouseMoveEvent boundedMoveShapes  current pos is out of image" << endl;
		return;
	}

	QPoint offset = pos - prev_point;
	shapeData::PointsType points = shape_unit.getPoints();
	convertToViewPoints(points);
	bool outOfImage_moving = false;
	for (auto point : points)
	{
		if (outOfPixmap(point + offset)) //判断拖动后，是否有顶点超出了image的范围
		{
			outOfImage_moving = true;
			break;
		}
	}
	
	if (!outOfImage_moving)
	{
		float fScale = m_fScaleList[m_nScaleInx];
		shape_unit.moveBy(offset/ fScale, selected_vertex_index);
		prev_point = pos;
	}
	
}

void PaintWidget::selectShapePoint(QPoint pos)
{
	bool not_selected = true;
	std::for_each(all_shapes.begin(), all_shapes.end(), [=, &not_selected](shapeData& s)
	{
		if (s.IsVisibel())
		{
			if (containsPoint(s, pos))
			{
				not_selected = false;
				s.setSelected(true);
				setSelectedShape(s);
			}
			else
			{
				s.setSelected(false);
				if (selected_shape == s)
				{
					selected_shape.setSelected(false);
				}	
			}
		}
	});

	emit selectionChanged(not_selected, selected_shape);
}

bool PaintWidget::read_json(QString json_file, PaintWidget::ShapeDataType& all_data)
{
	QFile file(json_file);
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << u8"读取json文件出错";
		return false;
	}

	QByteArray data = file.readAll();
	file.close();

	QJsonParseError jsonpe;
	QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &jsonpe);
	if (jsonpe.error == QJsonParseError::NoError)
	{
		if (jsonDocument.isObject())   // JSON 文档为对象
		{
			QJsonObject object = jsonDocument.object();  // 转化为对象
			if (object.contains("shapes"))
			{
				QJsonValue value = object.value("shapes");
				if (value.isArray())//array对象
				{
					shapeData shape_unit;
					QJsonArray array = value.toArray();  // 转化为数组
					int nSize = array.size();  // 获取数组大小
					for (int i = 0; i < nSize; ++i)   // 遍历数组
					{
						shape_unit.clearData();
						QJsonValue value = array.at(i);
						if (value.isObject())
						{
							QJsonObject obj = value.toObject();
							if (obj.contains("label"))
							{
								QJsonValue data = obj.value("label");
								if (data.isString())
								{
									QString temp = data.toString();
									shape_unit.setLabel(temp);
								}
							}

							if (obj.contains("shape_type"))
							{
								QJsonValue data = obj.value("shape_type");
								if (data.isString())
								{
									QString temp = data.toString();
									shapeData::eShapeType shape_type = temp == "rectangle" ? shapeData::eShapeType::rectangle : shapeData::eShapeType::ploygon;
									shape_unit.setShapeType(shape_type);
								}
							}

							if (obj.contains("points"))
							{
								shapeData::PointsType points;
								QJsonValue data = obj.value("points");
								if (data.isArray())
								{
									QJsonArray points_array = data.toArray();

									int nSize = points_array.size();  // 获取数组大小
									for (int j = 0; j < nSize; ++j)
									{
										QJsonValue point_unit = points_array.at(j);
										if (point_unit.isArray())
										{
											int X = -1;
											int Y = -1;
											QJsonArray point_data = point_unit.toArray();
											if (point_data.size() >= 2)   // 遍历数组
											{
												QJsonValue x_data = point_data.at(0);
												QJsonValue y_data = point_data.at(1);
												if (x_data.isDouble())
												{
													X = x_data.toInt();
												}

												if (y_data.isDouble())
												{
													Y = y_data.toInt();
												}
											}

											if (X >= 0 && Y >= 0)
											{
												QPoint p(X, Y);
												points.push_back(p);
											}
										}// end point_unit
									}
	
								}//points array
								shape_unit.setPoints(points);
							}	
							all_data.push_back(shape_unit);
						}// end value object
					}//end for (int i = 0; i < nSize; ++i)
				}// end shapes array

				
			}
		}//end jsonDocument
	}

	return true;
}

QJsonObject PaintWidget::write_json(QString image_path, SingleLabelingData labeling_data)
{
	QJsonObject json_obj;
	//键值对
	json_obj.insert("version", "1.0.0");

	//flags对象
	QJsonObject flags;
	json_obj.insert("flags", flags);

	//shapes数组,成员是对象(复杂json)
	QJsonArray shapes;
	for (auto unit : labeling_data.single_data)
	{
		QJsonObject shape_unit;
		shape_unit.insert("label", unit.label);

		QJsonValue shape_line_color(QJsonValue::Null);
		shape_unit.insert("line_color", shape_line_color);
		shape_unit.insert("fill_color", shape_line_color);

		QJsonArray shape_points;
		int x = 0;
		int y = 0;
		for (auto point : unit.points)
		{
			x = point.x();
			y = point.y();
			QJsonArray point_unit;
			point_unit.push_back(x);
			point_unit.push_back(y);
			shape_points.push_back(point_unit);
		}
		shape_unit.insert("points", shape_points);

		QString shapeType = unit.shape_type == shapeData::eShapeType::rectangle ? "rectangle" : "polygon";
		shape_unit.insert("shape_type", shapeType);

		shapes.push_back(shape_unit);
	}
	json_obj.insert("shapes", shapes);

	//lineColor数组
	QJsonArray line_color;
	line_color.push_back(255);
	line_color.push_back(0);
	line_color.push_back(0);
	line_color.push_back(128);
	json_obj.insert("lineColor", line_color);

	//fillColor数组
	QJsonArray fill_color;
	fill_color.push_back(255);
	fill_color.push_back(0);
	fill_color.push_back(0);
	fill_color.push_back(128);
	json_obj.insert("fillColor", fill_color);

	//键值对
	json_obj.insert("imagePath", image_path);

	//null数据
	QJsonValue image_data(QJsonValue::Null);
	json_obj.insert("imageData", image_data);

	//键值对
	json_obj.insert("imageHeight", image.height());
	json_obj.insert("imageWidth", image.width());
	return json_obj;
}

QJsonObject PaintWidget::write_json(QString image_path, QStringList labels)
{
	QJsonObject json_obj;
	//键值对
	json_obj.insert("version", "1.0.0");

	//flags对象
	QJsonObject flags;
	bool cmp = false;
	for (auto label : labels)
	{
		cmp = label == image_label ? true : false;
		flags.insert(label, cmp);
	}
	json_obj.insert("flags", flags);

	//shapes数组,成员是对象(复杂json)
	QJsonArray shapes;
	json_obj.insert("shapes", shapes);

	//lineColor数组
	QJsonArray line_color;
	line_color.push_back(255);
	line_color.push_back(0);
	line_color.push_back(0);
	line_color.push_back(128);
	json_obj.insert("lineColor", line_color);

	//fillColor数组
	QJsonArray fill_color;
	fill_color.push_back(255);
	fill_color.push_back(0);
	fill_color.push_back(0);
	fill_color.push_back(128);
	json_obj.insert("fillColor", fill_color);

	//键值对
	json_obj.insert("imagePath", image_path);

	//null数据
	QJsonValue image_data(QJsonValue::Null);
	json_obj.insert("imageData", image_data);

	//键值对
	json_obj.insert("imageHeight", image.height());
	json_obj.insert("imageWidth", image.width());
	return json_obj;
}

bool PaintWidget::outOfPixmap(QPoint p)
{
	int w = width();
	int h = height();

	bool j_x = (0 <= p.x() && p.x() < w);
	bool j_y = (0 <= p.y() && p.y() < h);
	return !(j_x && j_y);
}

void PaintWidget::ZoomRequest(double nScaleDelta)
{
	//qDebug() << nScaleDelta;
	//std::for_each(all_shapes.begin(), all_shapes.end(), [=](shapeData& s)
	//{
	//	s.setPoints(nScaleDelta);
	//});
}

void PaintWidget::convertToViewPoints(shapeData::PointsType& points)
{
	float fScale = m_fScaleList[m_nScaleInx];
	std::for_each(points.begin(), points.end(), [=](QPoint & pt)
	{
		pt *= fScale;
	});
}

void PaintWidget::setOriginalScale() 
{
	m_nScaleInx = 0; 
	update();
}

bool PaintWidget::IsDataChanged()
{
	bool ret = false;
	if (image_file_name.isEmpty())
	{
		return ret;
	}

	QString json_name;
	if (I_MainWindow->IsExistLabelData(image_file_name, json_name)) //判断与当前图片同名的json文件是否存在
	{
		//解析json文件
		PaintWidget::ShapeDataType shapes;
		read_json(json_name, shapes);
		if (shapes != all_shapes) //两个vector直接比较，需要重载==
			ret = true;
	}
	else
	{
		if (all_shapes.size() > 0)
			ret = true;
	}
	return ret;
}

SingleLabelingData PaintWidget::getLabelingData()
{
	SingleLabelingData ret_data;
	std::for_each(all_shapes.begin(), all_shapes.end(), [=, &ret_data](shapeData& s)
	{
		LabelingDataUnit unit = getLabelingDataUnitFromShapeData(s);
		ret_data.addData(unit);
	});

	return ret_data;
}

LabelingDataUnit PaintWidget::getLabelingDataUnitFromShapeData(shapeData s)
{
	LabelingDataUnit ret_unit;
	ret_unit.shape_type = s.getShapeType();
	ret_unit.label = s.getLabel();
	ret_unit.points = s.getPoints();

	return ret_unit;
}

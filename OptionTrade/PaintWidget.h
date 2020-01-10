#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QtGui>
#include <QtWidgets>
#include <QMainWindow>
#include <QJsonObject>

#include "ShapeData.h"
#include "LabelingData.h"

class ImageProcessing;
class PaintWidget :public QLabel
{
	Q_OBJECT
public:
	explicit PaintWidget(QWidget *parent, ImageProcessing *p = nullptr);
	enum shape {
		Pen = 1,Line,Ellipse,Circle, Triangle, Rhombus, 
		Rect, Square, Hexagon, Null
	};
	void paint(QImage &theImage);
	void setImageName(QString image_name);
	void setImage(QImage img);
	QImage getImage(); // 外部获取图像

	void setShape(PaintWidget::shape ); 					// 设置绘制类型
	void setPenWidth(int w);								// 设置画笔宽度
	void setPenColor(QColor c);								// 设置画笔颜色
	void setOriginalScale();                                // 设置缩放比例为0，图片还原显示
	//-------标定相关----------------
	void setImageLabel(QString label) { image_label = label; }

	using ShapeDataType = std::vector<shapeData>;

	enum eMode
	{
		MODE_NONE,
		MODE_CREATE,
		MODE_EDIT
	};
	eMode getDrawMode() { return draw_mode; }
	void setDrawMode(eMode draw_mode_) { draw_mode = draw_mode_; }
	void setDrawType(shapeData::eShapeType draw_type_) { draw_type = draw_type_; }

	void setBtnOk(bool btn_ok) { bBtnOk = btn_ok; }
	void setBtnCancel(bool btn_cancel) { bBtnCancel = btn_cancel; }

	void setSelectedShape(const shapeData & shape_unit) { selected_shape = shape_unit; }

	void overrideCursor(QCursor cursor) { setCursor(cursor); }

	//所有shape的最大index值
	int getMaxIndex();
	bool get_shape_from_item(int index, shapeData & shape_unit);
	void addShape(const shapeData& shape_unit);
	void updateShape(const shapeData& shape_unit);
	void updateLabel(int index, QString label);

	//进入创建模式后，要清除选中shape的selected标志
	void clearAllSelectShape();
	void clearSelectShape();
	void clearAllShapeData();

	void removeShape(int index);
	bool getCurrentSelectedShape(shapeData & shape_unit);

	//检测/分割标定数据生成
	QJsonObject write_json(QString image_path, SingleLabelingData labeling_data);
	bool read_json(QString json_file, PaintWidget::ShapeDataType& all_data);

	//分类标定数据生成
	QJsonObject write_json(QString image_path, QStringList labels);

	void ZoomRequest(double nScaleDelta);

	//当前标定数据是否发生改变
	//标定文件存在，则文件中的数据与内存中的数据进行比较，不相等则表示标定数据发生了改变
	//标定文件不存在，而内存中存在数据，也表示标定数据发生了改变
	bool IsDataChanged();

	//当前图片的all_shapes，转换为标定数据
	SingleLabelingData getLabelingData();

signals:
	void newShape(shapeData& shape_unit);
	void selectionChanged(bool not_selected, const shapeData& shape_unit);

Q_SIGNALS:
	void Signal_Average(int nLeft, int nTop, int nRight, int nButton, bool bDelete = false);
	void Signal_MinMax(int nLeft, int nTop, int nRight, int nButton);
	void Signal_Zoom(double nScaleDelta);
	void Signal_MousePos(int nPosX, int nPosY);
	void Signal_RegionArea(QRect rect);
	void Signal_FivePointCheck(bool bcheck);
	
private slots:
	/******************************************************************************
	* 函数名:slot_action_DrawShape
	* 描述: 绘制图形
	* IN  :
	* OUT:
	* 返回值:
	******************************************************************************/
	void slot_action_DrawShape();
	
protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent*event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);

	virtual void wheelEvent(QWheelEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *ev) Q_DECL_OVERRIDE;
	
	//-------标定相关----------------
	bool IsDrawing() { return draw_mode == MODE_CREATE; }
	bool IsEditing() { return draw_mode == MODE_EDIT; }

	//fScale：显示坐标时的缩放因子
    //shape_object缓存的坐标是像素坐标
	void paintShape(shapeData shape_object, QColor line_color, QColor fill_color, float fScale=1);

	void finalise();

	//
	QPainterPath* makePath(shapeData shape_unit);
	bool containsPoint(shapeData shape_unit, QPoint point);
	bool nearestVertex(shapeData shape_unit, QPoint point, qreal epsilon, int& point_index);
	qreal Distance(QPoint p1, QPoint p2);

	QRectF getRectFromLine(QPoint p1, QPoint p2);

	//移动shape
	void boundedMoveShapes(shapeData &shape_unit, QPoint pos);
	void selectShapePoint(QPoint pos);

	//
	bool outOfPixmap(QPoint pos);

	//points：输入输出参数
	//输入为像素坐标点，乘以缩放因子，输出为视图坐标点
	void convertToViewPoints(shapeData::PointsType& points);

	LabelingDataUnit getLabelingDataUnitFromShapeData(shapeData s);
private:
	ImageProcessing *I_MainWindow;
	PaintWidget::shape type;
	int penWidth; 
	QColor penColor;
	QString image_file_name;
	QImage image;
	QImage tempImage;
	QPoint lastPoint;
	QPoint endPoint;
	bool isDrawing;

	bool m_bZoomCenter;// 视图中心放大
	bool m_bZoom;//勾选放大
	bool m_bleftZoom;//进入放大画框状态
	//JCRect m_tRectZoom;//画框区域
	QImage m_srcImage;//源图片
	bool m_bDrawZoomImage;//进入画放大图状态
	bool m_bFivePoint;//是否开启五点区域
	float m_fScaleList[14] = { 1,1.5,2,4,6,8,10,16,32,64,96,128,160,200 };
	int m_nScaleInx;
	bool m_bDrag;//默认中心放大，右键鼠标拖动
	QPoint m_tLastPoint;
	int m_nDragW; //拖动累计长度
	int m_nDragH;//拖动累计高度

	bool m_bMousePress = false;
	QPoint m_qpPreDot;
	int m_nActionType = None;         //动作(放大,缩小,移动...)
	QPoint m_qpCurrentOffset;  //一次的图片偏移值
	QPoint m_qpOffsetTotal;    //总偏移
	bool   m_bAmplification;
	int action = None;
	enum  Type
	{
		None = 0,
		Amplification,
		Shrink,
		Lift,
		Right,
		Up,
		Down,
		Move
	};
	Qt::CursorShape m_tLastCursorShape;

    //-------标定相关----------------
	eMode draw_mode;
	shapeData::eShapeType draw_type;

	bool bBtnOk;
	bool bBtnCancel;
	bool move_shape;  //表示是否拖动框

	shapeData line;
	shapeData current_shape;
	ShapeDataType all_shapes;

	QPoint prev_point;    //移动前的位置，用于move
	shapeData selected_shape; //选中的shape，用于move
	shapeData h_shape; //缓存选中顶点vertex时shape数据，拖动顶点时使用
	int selected_vertex_index;  //对应hVertex  -1 表示没有选中顶点 >=0表示选中的顶点在shape中的index（shape中缓存各个顶点的是points，一个vector）

	QString image_label; //存储分类任务中图片的label
};

#endif// !PAINTWIDGET_H

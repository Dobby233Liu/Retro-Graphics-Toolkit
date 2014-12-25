#include "project.h"
#include "includes.h"
#include "classpalettebar.h"
#include "color_convert.h"
#include "callbacks_palette.h"
#include <stdexcept>
static const char*namesGen[]={"Red","Green","Blue"};
static const char*namesNES[]={"Hue","Value","Emphasis"};
paletteBar palBar;
void paletteBar::addTab(unsigned tab,bool all,bool tiny,bool alt){
	this->all[tab]=all;
	this->tiny[tab]=tiny;
	this->alt[tab]=alt;
	unsigned offsetx=16;
	unsigned offsety=tiny?54:56;
	ox[tab]=baseOffx[tab]=offsetx;
	oy[tab]=baseOffy[tab]=offsety;
	sysCache=currentProject->gameSystem;
	for(unsigned i=0;i<3;++i){
		slide[tab][i]=new Fl_Hor_Value_Slider(offsetx+32,offsety+((all?4:1)*(tiny?26:32)),tiny?128:256,tiny?22:24,namesGen[i]);
		slide[tab][i]->minimum(0);
		slide[tab][i]->maximum(7);
		slide[tab][i]->step(1);
		slide[tab][i]->value(0);
		slide[tab][i]->align(FL_ALIGN_LEFT);
		slide[tab][i]->callback(update_palette, (void*)i);
		offsety+=tiny?6:8;
		offsety+=tiny?26:32;
	}
}
void paletteBar::setSys(void){
	if(sysCache!=currentProject->gameSystem){
		sysCache=currentProject->gameSystem;
		for(unsigned j=0;j<tabsWithPalette;++j){
			for(unsigned i=0;i<3;++i){
				switch(currentProject->gameSystem){
					case sega_genesis:
						slide[j][i]->label(namesGen[i]);
						slide[j][i]->maximum(7);
					break;
					case NES:
						slide[j][i]->label(namesNES[i]);
					break;
				}
			}
			switch(currentProject->gameSystem){
				case sega_genesis:
					slide[j][1]->labelsize(13);
					slide[j][2]->labelsize(14);
					slide[j][2]->resize(slide[j][2]->x()-16,slide[j][2]->y(),slide[j][2]->w()+16,slide[j][2]->h());
					slide[j][2]->callback(update_palette, (void*)2);
				break;
				case NES:
					slide[j][0]->maximum(15);
					slide[j][1]->maximum(3);
					slide[j][1]->labelsize(14);
					slide[j][2]->labelsize(12);
					slide[j][2]->value(0);
					slide[j][2]->maximum(7);
					slide[j][2]->resize(slide[j][2]->x()+16,slide[j][2]->y(),slide[j][2]->w()-16,slide[j][2]->h());
					slide[j][2]->callback(update_emphesis);
				break;
			}
			selBox[j]%=currentProject->pal->perRow;
		}
	}
	updateSliders();
}
void paletteBar::updateSize(unsigned tab){
	ox[tab]=(float)((float)window->w()/800.f)*(float)baseOffx[tab];
	oy[tab]=(float)((float)window->h()/600.f)*(float)baseOffy[tab];
}
void paletteBar::updateSlider(unsigned tab){
	if(currentProject->pal->palType[selBox[tab]+(selRow[tab]*currentProject->pal->perRow)]){
		for(unsigned i=0;i<3;++i)
			slide[tab][i]->hide();
	}else{
		for(unsigned i=0;i<3;++i)
			slide[tab][i]->show();
		switch (currentProject->gameSystem){
			case sega_genesis:
				slide[tab][2]->value(currentProject->pal->palDat[(selBox[tab]*2)+(selRow[tab]*32)]);
				slide[tab][1]->value(currentProject->pal->palDat[1+(selBox[tab]*2)+(selRow[tab]*32)]>>4);
				slide[tab][0]->value(currentProject->pal->palDat[1+(selBox[tab]*2)+(selRow[tab]*32)]&15);		
			break;
			case NES:
				if(alt[tab]){
					slide[tab][0]->value(currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)+16]&15);
					slide[tab][1]->value((currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)+16]>>4)&3);
				}else{
					slide[tab][0]->value(currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)]&15);
					slide[tab][1]->value((currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)]>>4)&3);
				}
			break;
			default:
				show_default_error
		}
	}
	window->palType[currentProject->pal->palType[selBox[tab]+(selRow[tab]*currentProject->pal->perRow)]]->setonly();
	window->palType[currentProject->pal->palType[selBox[tab]+(selRow[tab]*currentProject->pal->perRow)]+3]->setonly();
	window->palType[currentProject->pal->palType[selBox[tab]+(selRow[tab]*currentProject->pal->perRow)]+6]->setonly();
}
void paletteBar::drawBoxes(unsigned tab){
	unsigned box_size=window->pal_size->value();
	unsigned x,y,a;
	a=currentProject->pal->perRow*3;
	if(all[tab]){
		unsigned loc_x,loc_y;
		loc_x=(float)((float)window->w()/800.f)*(float)palette_preview_box_x;
		loc_y=(float)((float)window->h()/600.f)*(float)palette_preview_box_y;
		fl_rectf(loc_x,loc_y,box_size*4,box_size*4,currentProject->pal->rgbPal[(selBox[tab]*3)+(selRow[tab]*a)],currentProject->pal->rgbPal[(selBox[tab]*3)+(selRow[tab]*a)+1],currentProject->pal->rgbPal[(selBox[tab]*3)+(selRow[tab]*a)+2]);//this will show larger preview of current color
	}
	if(!all[tab]){
		uint8_t*rgbPtr=currentProject->pal->rgbPal+(a*selRow[tab]);
		if(alt&&(currentProject->gameSystem==NES))
			rgbPtr+=currentProject->pal->colorCnt*3;
		for (x=0;x<currentProject->pal->perRow;++x){
			fl_rectf(ox[tab]+(x*box_size),oy[tab],box_size,box_size,*rgbPtr,*(rgbPtr+1),*(rgbPtr+2));
			rgbPtr+=3;
		}
		fl_draw_box(FL_EMBOSSED_FRAME,selBox[tab]*box_size+ox[tab],oy[tab],box_size,box_size,0);
	}else{
		uint8_t*rgbPtr=currentProject->pal->rgbPal;
		if(alt&&(currentProject->gameSystem==NES))
			rgbPtr+=currentProject->pal->colorCnt*3;
		for (y=0;y<currentProject->pal->rowCntPal;++y){
			for (x=0;x<currentProject->pal->perRow;++x){
				fl_rectf(ox[tab]+(x*box_size),oy[tab]+(y*box_size),box_size,box_size,*rgbPtr,*(rgbPtr+1),*(rgbPtr+2));
				rgbPtr+=3;
			}
		}
		fl_draw_box(FL_EMBOSSED_FRAME,selBox[tab]*box_size+ox[tab],selRow[tab]*box_size+oy[tab],box_size,box_size,0);
	}
}
unsigned paletteBar::toTab(unsigned realtab){
	static const int tabLut[]={0,1,2,-1,3,-1,-1};
	if(tabLut[realtab]<0)
		throw std::invalid_argument("Invalid tab");
	return tabLut[realtab];
}
void paletteBar::checkBox(int x,int y,unsigned tab){
	/*!
	This function is in charge of seeing if the mouse click is on a box and what box it is
	for x and y pass the mouser coordinate
	*/
	unsigned boxSize=window->pal_size->value();
	x-=ox[tab];
	y-=oy[tab];
	if (x < 0)
		return;
	if (y < 0)
		return;
	x/=boxSize;
	if (x >= currentProject->pal->perRow)
		return;
	y/=boxSize;
	if (y >= (all?currentProject->pal->rowCntPal:1))
		return;
	selBox[tab]=x;
	if(all[tab])
		changeRow(y,tab);
	else
		updateSlider(tab);
	window->redraw();
}

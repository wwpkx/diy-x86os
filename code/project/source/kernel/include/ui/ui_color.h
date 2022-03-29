/**
 * 32位颜色值
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_COLOR_H
#define OS_COLOR_H

#include <core/types.h>

// 参见：http://www.ilikeseo.cn/wangzhanyingxiaozhishi_30.html
#define  COLOR_LightPink      0xFFB6C1   // ǳ�ۺ�
#define  COLOR_Pink       0xFFC0CB   // �ۺ�
#define  COLOR_Crimson        0xDC143C   // ���/�ɺ�
#define  COLOR_LavenderBlush      0xFFF0F5   // ���Ϻ�
#define  COLOR_PaleVioletRed      0xDB7093   // ����������
#define  COLOR_HotPink        0xFF69B4       // ����ķۺ�
#define  COLOR_DeepPink       0xFF1493    // ��ۺ�
#define  COLOR_MediumVioletRed        0xC71585    // ����������
#define  COLOR_Orchid         0xDA70D6   // ����ɫ/������
#define  COLOR_Thistle        0xD8BFD8   // ��ɫ
#define  COLOR_Plum       0xDDA0DD   // ����ɫ/������
#define  COLOR_Violet         0xEE82EE   // ������
#define  COLOR_Magenta        0xFF00FF   // ���/õ���
#define  COLOR_Fuchsia        0xFF00FF   // �Ϻ�/��������
#define  COLOR_DarkMagenta        0x8B008B    // �����
#define  COLOR_Purple         0x800080   // ��ɫ
#define  COLOR_MediumOrchid       0xBA55D3    // ��������
#define  COLOR_DarkViolet         0x9400D3    // ��������
#define  COLOR_DarkOrchid         0x9932CC    // ��������
#define  COLOR_Indigo         0x4B0082   // ����/����ɫ
#define  COLOR_BlueViolet         0x8A2BE2    // ��������
#define  COLOR_MediumPurple       0x9370DB    // ����ɫ
#define  COLOR_MediumSlateBlue        0x7B68EE    // �а���ɫ/�а�����
#define  COLOR_SlateBlue      0x6A5ACD   // ʯ��ɫ/������
#define  COLOR_DarkSlateBlue      0x483D8B    // ������ɫ/��������
#define  COLOR_Lavender       0xE6E6FA    // ����ɫ/Ѭ�²ݵ���
#define  COLOR_GhostWhite         0xF8F8FF    // �����
#define  COLOR_Blue       0x0000FF   // ����
#define  COLOR_MediumBlue         0x0000CD    // ����ɫ
#define  COLOR_MidnightBlue       0x191970    // ��ҹ��
#define  COLOR_DarkBlue       0x00008B    // ����ɫ
#define  COLOR_Navy       0x000080   // ������
#define  COLOR_RoyalBlue      0x4169E1    // �ʼ���/����
#define  COLOR_CornflowerBlue         0x6495ED    // ʸ������
#define  COLOR_LightSteelBlue         0xB0C4DE    // ������
#define  COLOR_LightSlateGray         0x778899    // ������/��ʯ���
#define  COLOR_SlateGray      0x708090    // ��ʯɫ/ʯ���
#define  COLOR_DodgerBlue         0x1E90FF    // ����ɫ/������
#define  COLOR_AliceBlue      0xF0F8FF    // ����˿��
#define  COLOR_SteelBlue      0x4682B4    // ����/����
#define  COLOR_LightSkyBlue       0x87CEFA    // ������ɫ
#define  COLOR_SkyBlue        0x87CEEB   // ����ɫ
#define  COLOR_DeepSkyBlue        0x00BFFF   // ������
#define  COLOR_LightBlue      0xADD8E6   // ����
#define  COLOR_PowderBlue         0xB0E0E6   // ����ɫ/��ҩ��
#define  COLOR_CadetBlue      0x5F9EA0   // ����ɫ/������
#define  COLOR_Azure      0xF0FFFF   // ε��ɫ
#define  COLOR_LightCyan      0xE0FFFF    // ����ɫ
#define  COLOR_PaleTurquoise      0xAFEEEE    // ���̱�ʯ
#define  COLOR_Cyan       0x00FFFF   // ��ɫ
#define  COLOR_Aqua       0x00FFFF   // ǳ��ɫ/ˮɫ
#define  COLOR_DarkTurquoise      0x00CED1    // ���̱�ʯ
#define  COLOR_DarkSlateGray      0x2F4F4F    // ���߻�ɫ/��ʯ���
#define  COLOR_DarkCyan       0x008B8B    // ����ɫ
#define  COLOR_Teal       0x008080   // ˮѼɫ
#define  COLOR_MediumTurquoise        0x48D1CC    // ���̱�ʯ
#define  COLOR_LightSeaGreen      0x20B2AA    // ǳ������
#define  COLOR_Turquoise      0x40E0D0   // �̱�ʯ
#define  COLOR_Aquamarine         0x7FFFD4   // ��ʯ����
#define  COLOR_MediumAquamarine       0x66CDAA    // �б�ʯ����
#define  COLOR_MediumSpringGreen      0x00FA9A    // �д���ɫ
#define  COLOR_MintCream      0xF5FFFA   // ��������
#define  COLOR_SpringGreen        0x00FF7F   // ����ɫ
#define  COLOR_MediumSeaGreen         0x3CB371    // �к�����
#define  COLOR_SeaGreen       0x2E8B57    // ������
#define  COLOR_Honeydew       0xF0FFF0    // ��ɫ/�۹�ɫ
#define  COLOR_LightGreen         0x90EE90    // ����ɫ
#define  COLOR_PaleGreen      0x98FB98    // ����ɫ
#define  COLOR_DarkSeaGreen       0x8FBC8F    // ��������
#define  COLOR_LimeGreen      0x32CD32    // ��������
#define  COLOR_Lime       0x00FF00   // ������
#define  COLOR_ForestGreen        0x228B22    // ɭ����
#define  COLOR_Green      0x008000   // ����
#define  COLOR_DarkGreen      0x006400   // ����ɫ
#define  COLOR_Chartreuse         0x7FFF00   // ����ɫ/���ؾ���
#define  COLOR_LawnGreen      0x7CFC00   // ����ɫ/��ƺ��
#define  COLOR_GreenYellow        0xADFF2F   // �̻�ɫ
#define  COLOR_DarkOliveGreen         0x556B2F    // �������
#define  COLOR_YellowGreen        0x9ACD32   // ����ɫ
#define  COLOR_OliveDrab      0x6B8E23   // ��魺�ɫ
#define  COLOR_Beige      0xF5F5DC   // ��ɫ/����ɫ
#define  COLOR_LightGoldenrodYellow       0xFAFAD2    // ���ջ�
#define  COLOR_Ivory      0xFFFFF0   // ����ɫ
#define  COLOR_LightYellow        0xFFFFE0    // ǳ��ɫ
#define  COLOR_Yellow         0xFFFF00   // ����
#define  COLOR_Olive      0x808000   // ���
#define  COLOR_DarkKhaki      0xBDB76B    // ���ƺ�ɫ/�ߴ��
#define  COLOR_LemonChiffon       0xFFFACD   // ���ʳ�
#define  COLOR_PaleGoldenrod      0xEEE8AA   // �Ҿջ�/������ɫ
#define  COLOR_Khaki      0xF0E68C   // �ƺ�ɫ/��ߴ��
#define  COLOR_Gold       0xFFD700   // ��ɫ
#define  COLOR_Cornsilk       0xFFF8DC    // ����˿ɫ
#define  COLOR_Goldenrod      0xDAA520   // ��ջ�
#define  COLOR_DarkGoldenrod      0xB8860B    // ����ջ�
#define  COLOR_FloralWhite        0xFFFAF0    // ���İ�ɫ
#define  COLOR_OldLace        0xFDF5E6   // �ϻ�ɫ/����˿
#define  COLOR_Wheat      0xF5DEB3   // ǳ��ɫ/С��ɫ
#define  COLOR_Moccasin       0xFFE4B5    // ¹Ƥɫ/¹Ƥѥ
#define  COLOR_Orange         0xFFA500   // ��ɫ
#define  COLOR_PapayaWhip         0xFFEFD5    // ��ľɫ/��ľ��
#define  COLOR_BlanchedAlmond         0xFFEBCD    // ����ɫ
#define  COLOR_NavajoWhite        0xFFDEAD   // ���߰�/������
#define  COLOR_AntiqueWhite       0xFAEBD7    // �Ŷ���
#define  COLOR_Tan        0xD2B48C
#define  COLOR_BurlyWood      0xDEB887    // Ӳľɫ
#define  COLOR_Bisque         0xFFE4C4   // ������
#define  COLOR_DarkOrange         0xFF8C00    // ���ɫ
#define  COLOR_Linen      0xFAF0E6   // ���鲼
#define  COLOR_Peru       0xCD853F   // ��³ɫ
#define  COLOR_PeachPuff      0xFFDAB9    // ����ɫ
#define  COLOR_SandyBrown         0xF4A460    // ɳ��ɫ
#define  COLOR_Chocolate      0xD2691E    // �ɿ���ɫ
#define  COLOR_SaddleBrown        0x8B4513    // �غ�ɫ/����ɫ
#define  COLOR_Seashell       0xFFF5EE    // ������
#define  COLOR_Sienna         0xA0522D   // ������ɫ
#define  COLOR_LightSalmon        0xFFA07A    // ǳ������ɫ
#define  COLOR_Coral      0xFF7F50   // ɺ��
#define  COLOR_OrangeRed      0xFF4500    // �Ⱥ�ɫ
#define  COLOR_DarkSalmon         0xE9967A    // ������/����ɫ
#define  COLOR_Tomato         0xFF6347   // ���Ѻ�
#define  COLOR_MistyRose      0xFFE4E1    // ǳõ��ɫ/����õ��
#define  COLOR_Salmon         0xFA8072   // ����/����ɫ
#define  COLOR_Snow       0xFFFAFA   // ѩ��ɫ
#define  COLOR_LightCoral         0xF08080    // ��ɺ��ɫ
#define  COLOR_RosyBrown      0xBC8F8F    // õ����ɫ
#define  COLOR_IndianRed      0xCD5C5C    // ӡ�Ⱥ�
#define  COLOR_Red        0xFF0000   // ��
#define  COLOR_Brown      0xA52A2A   // ��ɫ
#define  COLOR_FireBrick      0xB22222    // ��שɫ/�ͻ�ש
#define  COLOR_DarkRed        0x8B0000   // ���ɫ
#define  COLOR_Maroon         0x800000   // ��ɫ
#define  COLOR_White      0xFFFFFF   // ����
#define  COLOR_WhiteSmoke         0xF5F5F5    // ����
#define  COLOR_Gainsboro      0xDCDCDC    // ����ɫ
#define  COLOR_LightGrey      0xD3D3D3    // ǳ��ɫ
#define  COLOR_Silver         0xC0C0C0   // ����ɫ
#define  COLOR_DarkGray       0xA9A9A9    // ���ɫ
#define  COLOR_Gray       0x808080   // ��ɫ
#define  COLOR_DimGray        0x696969   // ������
#define  COLOR_Black      0x000000   // ��

typedef uint32_t ui_color_t;

#endif //OS_COLOR_H

sample_mono_display
��������������ʾ��ģʽ��Display Manager(DISOMNG)������ʾ
�����ʽ��sample_mono_display
����˵������
����ο���sample_mono_display
ע���������ʾ֮ǰ����Ҫ�����ӻ����ӵ������е�HDMI1�˿��ϡ�

sample_dual_display
����������˫����ʾ��չʾ�����е�˫����ʾ����
�����ʽ��sample_dual_display
����˵������
����ο���sample_dual_display
ע���������ʾ֮ǰ����Ҫ����̨���ӻ����ӵ������е�����HDMI�˿��ϡ�

sample_display_mode
��������:��ʾ��ʾģʽ������ع��ܣ�֧�����������У�
           help       ������Ϣ
           info       չʾ������������Ϣ
           capa       չʾ����������Ϣ
           status     չʾ���ӻ�״̬��Ϣ
           modes      չʾ���ӻ�֧����ʾģʽ�б�
           mode       չʾ��ǰ����ʾģʽ
           enable     ʹ�ܵ�ǰ����ʾ�ӿ�
           disable    ȥʹ�ܵ�ǰ����ʾ�ӿ�
           offset [left] [top] [right] [bottom]  ������ʾ��ƫ��
           brightness [0~1023]    ��������
           contrast   [0~1023]    ���öԱȶ�
           hue        [0~1023]    ����ɫ��
           saturation [0~1023]    ���ñ��Ͷ�
           auto       ����ʾģʽ����Ϊ�Զ�ģʽ
           format [format]   ������ʾģʽ������ģʽ��������ӻ��Ƿ�֧�֣�
           force  [format]   ������ʾģʽ��ǿ��ģʽ���������ӻ��Ƿ�֧�֣�
           rgb     �������pixel formatΪRGB
           444     �������pixel formatΪYUV444
           420     �������pixel formatΪYUV420
           422     �������pixel formatΪYUV422
           8       �������bit widthΪ8����
           10      �������bit widthΪ10����
           12      �������bit widthΪ12����
�����ʽ:
    sample_display_mode display_id
����˵��:
    display_id    ��ʾͨ��ID��0----������ʾͨ��, 1----������ʾͨ��
����ο�:
    ./sample_dual_dispaly 0
ע������:
    ����ʾ֮ǰ����Ҫ�����ӻ����ӵ���Ӧ��HDMI�ӿڡ�

sample_tsplay_multiscreen
��������:
    ��ʾ�ڲ�ͬ��֮�ϵ���Ƶ�Ĳ���
�����ʽ:
    sample_display_mode file|dir display_id [format]
����˵��:
    file|dir     ������tsý���ļ���Ŀ¼
    display_id   ��ʾͨ��ID��0----������ʾͨ��, 1----������ʾͨ��
    format       ��ʾ��ʽ����4320P120|4320P60|2160P60|2160P50|1080P60|1080P50
����ο�:
    ./sample_dual_dispaly /mnt/sdr.ts 0 1080P60
ע������:
    ����ʾ֮ǰ����Ҫ�����ӻ����ӵ���Ӧ��HDMI�ӿڡ�
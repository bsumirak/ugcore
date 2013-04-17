//  This file is parsed from UG 3.9.
//  It provides the Gauss Quadratures for a reference quadrilateral.


#include "../quadrature.h"
#include "gauss_quad_quadrilateral.h"

namespace ug{

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 1>, 2, 1, 1>::m_vWeight[1] =
{
 1.00000000000000000000000000000000
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 1>, 2, 1, 1>::m_vPoint[1] =
{
MathVector<2>(0.50000000000000000000000000000000, 0.50000000000000000000000000000000)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 2>, 2, 2, 4>::m_vWeight[4] =
{
 0.25000000000000000000000000000000,
 0.25000000000000000000000000000000,
 0.25000000000000000000000000000000,
 0.25000000000000000000000000000000
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 2>, 2, 2, 4>::m_vPoint[4] =
{
MathVector<2>(0.21132486540518700000000000000000, 0.21132486540518700000000000000000),
MathVector<2>(0.78867513459481300000000000000000, 0.21132486540518700000000000000000),
MathVector<2>(0.21132486540518700000000000000000, 0.78867513459481300000000000000000),
MathVector<2>(0.78867513459481300000000000000000, 0.78867513459481300000000000000000)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 3>, 2, 3, 4>::m_vWeight[4] =
{
 0.25000000000000000000000000000000,
 0.25000000000000000000000000000000,
 0.25000000000000000000000000000000,
 0.25000000000000000000000000000000
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 3>, 2, 3, 4>::m_vPoint[4] =
{
MathVector<2>(0.09175170953613698363378598754902, 0.50000000000000000000000000000000),
MathVector<2>(0.50000000000000000000000000000000, 0.09175170953613698363378598754902),
MathVector<2>(0.50000000000000000000000000000000, 0.90824829046386301636621401245098),
MathVector<2>(0.90824829046386301636621401245098, 0.50000000000000000000000000000000)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 4>, 2, 4, 6>::m_vWeight[6] =
{
 0.28571428571428571428571428571428,
 0.10989010989010989010989010989011,
 0.14151805175188302631601261486295,
 0.14151805175188302631601261486295,
 0.16067975044591917148618518733485,
 0.16067975044591917148618518733485
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 4>, 2, 4, 6>::m_vPoint[6] =
{
MathVector<2>(0.50000000000000000000000000000000, 0.50000000000000000000000000000000),
MathVector<2>(0.98304589153964795245728880523899, 0.50000000000000000000000000000000),
MathVector<2>(0.72780186391809642112479237299488, 0.07404267334769975434908217981667),
MathVector<2>(0.72780186391809642112479237299488, 0.92595732665230024565091782018333),
MathVector<2>(0.13418502421343273531598225407969, 0.18454360551162298687829339850317),
MathVector<2>(0.13418502421343273531598225407969, 0.81545639448837701312170660149683)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 5>, 2, 5, 7>::m_vWeight[7] =
{
 0.28571428571428571428571428571428,
 0.07936507936507936507936507936508,
 0.07936507936507936507936507936508,
 0.13888888888888888888888888888889,
 0.13888888888888888888888888888889,
 0.13888888888888888888888888888889,
 0.13888888888888888888888888888889
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 5>, 2, 5, 7>::m_vPoint[7] =
{
MathVector<2>(0.50000000000000000000000000000000, 0.50000000000000000000000000000000),
MathVector<2>(0.01695410846035204754271119476101, 0.50000000000000000000000000000000),
MathVector<2>(0.98304589153964795245728880523899, 0.50000000000000000000000000000000),
MathVector<2>(0.21132486540518711774542560974902, 0.11270166537925831148207346002176),
MathVector<2>(0.21132486540518711774542560974902, 0.88729833462074168851792653997824),
MathVector<2>(0.78867513459481288225457439025098, 0.11270166537925831148207346002176),
MathVector<2>(0.78867513459481288225457439025098, 0.88729833462074168851792653997824)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 6>, 2, 6, 10>::m_vWeight[10] =
{
 0.11383581142854335963838543032803,
 0.20684899330074137308917423457476,
 0.03600022114991134623215077164016,
 0.03600022114991134623215077164016,
 0.16706477606566628748245813610296,
 0.16706477606566628748245813610296,
 0.05636850122266983935276391196381,
 0.05636850122266983935276391196381,
 0.08022409919711016056884734784167,
 0.08022409919711016056884734784167
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 6>, 2, 6, 10>::m_vPoint[10] =
{
MathVector<2>(0.91820281684881280226895072035422, 0.50000000000000000000000000000000),
MathVector<2>(0.32126991730434640755691280777702, 0.50000000000000000000000000000000),
MathVector<2>(0.93605076559656529501643767018721, 0.05561799267261773307341476419531),
MathVector<2>(0.93605076559656529501643767018721, 0.94438200732738226692658523580469),
MathVector<2>(0.65299258107771333074469136784128, 0.19757118026765748615073385233221),
MathVector<2>(0.65299258107771333074469136784128, 0.80242881973234251384926614766779),
MathVector<2>(0.29486455026667108063715992772201, 0.02227624667946812612123581961261),
MathVector<2>(0.29486455026667108063715992772201, 0.97772375332053187387876418038739),
MathVector<2>(0.06356534442156033037437039575184, 0.21727000328062297705281230543027),
MathVector<2>(0.06356534442156033037437039575184, 0.78272999671937702294718769456973)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 7>, 2, 7, 12>::m_vWeight[12] =
{
 0.06049382716049382716049382716049,
 0.06049382716049382716049382716049,
 0.06049382716049382716049382716049,
 0.06049382716049382716049382716049,
 0.13014822916684861428497985801168,
 0.13014822916684861428497985801168,
 0.13014822916684861428497985801168,
 0.13014822916684861428497985801168,
 0.05935794367265755855452631482782,
 0.05935794367265755855452631482782,
 0.05935794367265755855452631482782,
 0.05935794367265755855452631482782
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 7>, 2, 7, 12>::m_vPoint[12] =
{
MathVector<2>(0.03708995011372426921671661170800, 0.50000000000000000000000000000000),
MathVector<2>(0.50000000000000000000000000000000, 0.03708995011372426921671661170800),
MathVector<2>(0.50000000000000000000000000000000, 0.96291004988627573078328338829200),
MathVector<2>(0.96291004988627573078328338829200, 0.50000000000000000000000000000000),
MathVector<2>(0.30972278339584217181044682045680, 0.30972278339584217181044682045680),
MathVector<2>(0.30972278339584217181044682045680, 0.69027721660415782818955317954320),
MathVector<2>(0.69027721660415782818955317954320, 0.30972278339584217181044682045680),
MathVector<2>(0.69027721660415782818955317954320, 0.69027721660415782818955317954320),
MathVector<2>(0.09701010854070062814607190932463, 0.09701010854070062814607190932463),
MathVector<2>(0.09701010854070062814607190932463, 0.90298989145929937185392809067537),
MathVector<2>(0.90298989145929937185392809067537, 0.09701010854070062814607190932463),
MathVector<2>(0.90298989145929937185392809067537, 0.90298989145929937185392809067537)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 8>, 2, 8, 16>::m_vWeight[16] =
{
 0.01384117640535994306158400271374,
 0.10109734218151885333357243373724,
 0.13338665123815861383831921172309,
 0.02926354719668480160617959307044,
 0.03140360440343670103296426305953,
 0.03140360440343670103296426305953,
 0.03413614618339711683328699903541,
 0.03413614618339711683328699903541,
 0.12085211980281423777756638059528,
 0.12085211980281423777756638059528,
 0.06313212660738592334508524616913,
 0.06313212660738592334508524616913,
 0.09031558097054314228151690054444,
 0.09031558097054314228151690054444,
 0.02136606352156177280975258997395,
 0.02136606352156177280975258997395
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 8>, 2, 8, 16>::m_vPoint[16] =
{
MathVector<2>(0.50000000000000000000000000000000, 0.50000000000000000000000000000000),
MathVector<2>(0.87881458883025272376243091093078, 0.50000000000000000000000000000000),
MathVector<2>(0.38156407887214905330677888913682, 0.50000000000000000000000000000000),
MathVector<2>(0.00514103547773665934255258242844, 0.50000000000000000000000000000000),
MathVector<2>(0.97526047782283344179383397817322, 0.18045434754981517329838236798414),
MathVector<2>(0.97526047782283344179383397817322, 0.81954565245018482670161763201586),
MathVector<2>(0.83194136844281658918106215312182, 0.03146546153750477812388445955262),
MathVector<2>(0.83194136844281658918106215312182, 0.96853453846249522187611554044738),
MathVector<2>(0.65210534086205225452112163378099, 0.23145823472925316105117594700364),
MathVector<2>(0.65210534086205225452112163378099, 0.76854176527074683894882405299636),
MathVector<2>(0.38175164073194025967137326853460, 0.05640574677518768049015587550525),
MathVector<2>(0.38175164073194025967137326853460, 0.94359425322481231950984412449475),
MathVector<2>(0.15052326195671822003768421503399, 0.25265058966490183398785216925757),
MathVector<2>(0.15052326195671822003768421503399, 0.74734941033509816601214783074243),
MathVector<2>(0.04980461289421013321826332793971, 0.05125209086011623123297208006836),
MathVector<2>(0.04980461289421013321826332793971, 0.94874790913988376876702791993164)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 9>, 2, 9, 17>::m_vWeight[17] =
{
 0.13168724279835390946502057613169,
 0.02221984454254967674175272155552,
 0.02221984454254967674175272155552,
 0.02221984454254967674175272155552,
 0.02221984454254967674175272155552,
 0.02802490053239912131897473175636,
 0.02802490053239912131897473175636,
 0.02802490053239912131897473175636,
 0.02802490053239912131897473175636,
 0.09957060981551752381908392923745,
 0.09957060981551752381908392923745,
 0.09957060981551752381908392923745,
 0.09957060981551752381908392923745,
 0.06726283440994520075393347341775,
 0.06726283440994520075393347341775,
 0.06726283440994520075393347341775,
 0.06726283440994520075393347341775
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 9>, 2, 9, 17>::m_vPoint[17] =
{
MathVector<2>(0.50000000000000000000000000000000, 0.50000000000000000000000000000000),
MathVector<2>(0.01557501681901113964165482851083, 0.18465994013416557291337985962548),
MathVector<2>(0.18465994013416557291337985962548, 0.98442498318098886035834517148917),
MathVector<2>(0.81534005986583442708662014037452, 0.01557501681901113964165482851083),
MathVector<2>(0.98442498318098886035834517148917, 0.81534005986583442708662014037452),
MathVector<2>(0.03601917702021516629962331530815, 0.87513854998945026676799199119468),
MathVector<2>(0.12486145001054973323200800880532, 0.03601917702021516629962331530815),
MathVector<2>(0.87513854998945026676799199119468, 0.96398082297978483370037668469185),
MathVector<2>(0.96398082297978483370037668469185, 0.12486145001054973323200800880532),
MathVector<2>(0.23813208989278533198085643668043, 0.27333008943217640461860617920896),
MathVector<2>(0.27333008943217640461860617920896, 0.76186791010721466801914356331957),
MathVector<2>(0.72666991056782359538139382079104, 0.23813208989278533198085643668043),
MathVector<2>(0.76186791010721466801914356331957, 0.72666991056782359538139382079104),
MathVector<2>(0.07369213533316884612739266174083, 0.53810416409630858659081343369466),
MathVector<2>(0.46189583590369141340918656630534, 0.07369213533316884612739266174083),
MathVector<2>(0.53810416409630858659081343369466, 0.92630786466683115387260733825917),
MathVector<2>(0.92630786466683115387260733825917, 0.46189583590369141340918656630534)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 11>, 2, 11, 24>::m_vWeight[24] =
{
 0.01200519083768095364069079399439,
 0.01200519083768095364069079399439,
 0.01200519083768095364069079399439,
 0.01200519083768095364069079399439,
 0.01651783229113764891840877021239,
 0.01651783229113764891840877021239,
 0.01651783229113764891840877021239,
 0.01651783229113764891840877021239,
 0.02434669433966704104903010994989,
 0.02434669433966704104903010994989,
 0.02434669433966704104903010994989,
 0.02434669433966704104903010994989,
 0.05293408749973715012598291533906,
 0.05293408749973715012598291533906,
 0.05293408749973715012598291533906,
 0.05293408749973715012598291533906,
 0.05640651543221584685078950405212,
 0.05640651543221584685078950405212,
 0.05640651543221584685078950405212,
 0.05640651543221584685078950405212,
 0.08778967959956135941509790645214,
 0.08778967959956135941509790645214,
 0.08778967959956135941509790645214,
 0.08778967959956135941509790645214
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 11>, 2, 11, 24>::m_vPoint[24] =
{
MathVector<2>(0.00868038822957226352375425149800, 0.15096194772521621761176509691252),
MathVector<2>(0.15096194772521621761176509691252, 0.99131961177042773647624574850200),
MathVector<2>(0.84903805227478378238823490308748, 0.00868038822957226352375425149800),
MathVector<2>(0.99131961177042773647624574850200, 0.84903805227478378238823490308748),
MathVector<2>(0.03025680859163154639678381891505, 0.91288791795148196865113729264497),
MathVector<2>(0.08711208204851803134886270735503, 0.03025680859163154639678381891505),
MathVector<2>(0.91288791795148196865113729264497, 0.96974319140836845360321618108495),
MathVector<2>(0.96974319140836845360321618108495, 0.08711208204851803134886270735503),
MathVector<2>(0.02323023589923399207749786658801, 0.59429306935932097730016228409137),
MathVector<2>(0.40570693064067902269983771590863, 0.02323023589923399207749786658801),
MathVector<2>(0.59429306935932097730016228409137, 0.97676976410076600792250213341199),
MathVector<2>(0.97676976410076600792250213341199, 0.40570693064067902269983771590863),
MathVector<2>(0.09373972584759344975530870904385, 0.34218828354237290200719514179895),
MathVector<2>(0.34218828354237290200719514179895, 0.90626027415240655024469129095615),
MathVector<2>(0.65781171645762709799280485820105, 0.09373972584759344975530870904385),
MathVector<2>(0.90626027415240655024469129095615, 0.65781171645762709799280485820105),
MathVector<2>(0.14399904346233184672546705243812, 0.76266012518227388117081594357001),
MathVector<2>(0.23733987481772611882918405642999, 0.14399904346233184672546705243812),
MathVector<2>(0.76266012518227388117081594357001, 0.85600095653766815327453294756188),
MathVector<2>(0.85600095653766815327453294756188, 0.23733987481772611882918405642999),
MathVector<2>(0.28757637557566537469228494424402, 0.52082903595601118413677340226885),
MathVector<2>(0.47917096404398881586322659773115, 0.28757637557566537469228494424402),
MathVector<2>(0.52082903595601118413677340226885, 0.71242362442433462530771505575598),
MathVector<2>(0.71242362442433462530771505575598, 0.47917096404398881586322659773115)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 13>, 2, 13, 33>::m_vWeight[33] =
{
 0.07509552885780634034649828603289,
 0.00749795971612478291640341391448,
 0.00749795971612478291640341391448,
 0.00749795971612478291640341391448,
 0.00749795971612478291640341391448,
 0.00954360532927091741009259373739,
 0.00954360532927091741009259373739,
 0.00954360532927091741009259373739,
 0.00954360532927091741009259373739,
 0.01510623095443749517021639829856,
 0.01510623095443749517021639829856,
 0.01510623095443749517021639829856,
 0.01510623095443749517021639829856,
 0.01937318463327633483945688833978,
 0.01937318463327633483945688833978,
 0.01937318463327633483945688833978,
 0.01937318463327633483945688833978,
 0.02971116682514890027044978654411,
 0.02971116682514890027044978654411,
 0.02971116682514890027044978654411,
 0.02971116682514890027044978654411,
 0.03244088759250067782138598265494,
 0.03244088759250067782138598265494,
 0.03244088759250067782138598265494,
 0.03244088759250067782138598265494,
 0.05333539536429734735825107104518,
 0.05333539536429734735825107104518,
 0.05333539536429734735825107104518,
 0.05333539536429734735825107104518,
 0.06421768737049195912711929395734,
 0.06421768737049195912711929395734,
 0.06421768737049195912711929395734,
 0.06421768737049195912711929395734
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceQuadrilateral, 13>, 2, 13, 33>::m_vPoint[33] =
{
MathVector<2>(0.50000000000000000000000000000000, 0.50000000000000000000000000000000),
MathVector<2>(0.00825665878006386810726796759466, 0.88940485577720971125947535810003),
MathVector<2>(0.11059514422279028874052464189997, 0.00825665878006386810726796759466),
MathVector<2>(0.88940485577720971125947535810003, 0.99174334121993613189273203240534),
MathVector<2>(0.99174334121993613189273203240534, 0.11059514422279028874052464189997),
MathVector<2>(0.02135115010684631717185618761372, 0.07022199717918053570742071616782),
MathVector<2>(0.07022199717918053570742071616782, 0.97864884989315368282814381238628),
MathVector<2>(0.92977800282081946429257928383218, 0.02135115010684631717185618761372),
MathVector<2>(0.97864884989315368282814381238628, 0.92977800282081946429257928383218),
MathVector<2>(0.02053741485623257123011933490039, 0.56909172993123267687360059018315),
MathVector<2>(0.43090827006876732312639940981685, 0.02053741485623257123011933490039),
MathVector<2>(0.56909172993123267687360059018315, 0.97946258514376742876988066509961),
MathVector<2>(0.97946258514376742876988066509961, 0.43090827006876732312639940981685),
MathVector<2>(0.02933638706353738152539868321423, 0.30463189193526949966156599515720),
MathVector<2>(0.30463189193526949966156599515720, 0.97066361293646261847460131678577),
MathVector<2>(0.69536810806473050033843400484280, 0.02933638706353738152539868321423),
MathVector<2>(0.97066361293646261847460131678577, 0.69536810806473050033843400484280),
MathVector<2>(0.07496166315012571201561720649945, 0.73790431260913795253379642170171),
MathVector<2>(0.26209568739086204746620357829829, 0.07496166315012571201561720649945),
MathVector<2>(0.73790431260913795253379642170171, 0.92503833684987428798438279350055),
MathVector<2>(0.92503833684987428798438279350055, 0.26209568739086204746620357829829),
MathVector<2>(0.12209732171395928186435768276542, 0.17608918140649463397820348570608),
MathVector<2>(0.17608918140649463397820348570608, 0.87790267828604071813564231723458),
MathVector<2>(0.82391081859350536602179651429392, 0.12209732171395928186435768276542),
MathVector<2>(0.87790267828604071813564231723458, 0.82391081859350536602179651429392),
MathVector<2>(0.15187496075412529302050214748279, 0.46462924550177753189147032406290),
MathVector<2>(0.46462924550177753189147032406290, 0.84812503924587470697949785251721),
MathVector<2>(0.53537075449822246810852967593710, 0.15187496075412529302050214748279),
MathVector<2>(0.84812503924587470697949785251721, 0.53537075449822246810852967593710),
MathVector<2>(0.29534771915298057834772392987155, 0.67135827802020339470656353671843),
MathVector<2>(0.32864172197979660529343646328157, 0.29534771915298057834772392987155),
MathVector<2>(0.67135827802020339470656353671843, 0.70465228084701942165227607012845),
MathVector<2>(0.70465228084701942165227607012845, 0.32864172197979660529343646328157)
};




template <>
FlexGaussQuadrature<ReferenceQuadrilateral>::FlexGaussQuadrature(int order)
{
	switch(order)
	{
	case 0:
	case 1:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 1>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 1>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 1>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 1>::weights();
		break;

	case 2:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 2>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 2>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 2>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 2>::weights();
		break;

	case 3:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 3>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 3>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 3>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 3>::weights();
		break;

	case 4:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 4>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 4>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 4>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 4>::weights();
		break;

	case 5:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 5>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 5>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 5>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 5>::weights();
		break;

	case 6:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 6>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 6>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 6>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 6>::weights();
		break;

	case 7:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 7>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 7>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 7>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 7>::weights();
		break;

	case 8:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 8>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 8>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 8>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 8>::weights();
		break;

	case 9:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 9>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 9>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 9>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 9>::weights();
		break;

	case 10:
	case 11:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 11>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 11>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 11>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 11>::weights();
		break;

	case 12:
	case 13:
		m_order = GaussQuadrature<ReferenceQuadrilateral, 13>::order();
		m_numPoints = GaussQuadrature<ReferenceQuadrilateral, 13>::size();
		m_pvPoint = GaussQuadrature<ReferenceQuadrilateral, 13>::points();
		m_pvWeight = GaussQuadrature<ReferenceQuadrilateral, 13>::weights();
		break;

	default: UG_THROW("Order "<<order<<" not available for GaussQuadrature of quadrilateral.");
	}
}
}; // namespace ug


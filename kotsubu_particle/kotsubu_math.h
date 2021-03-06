/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_math v1.0

・概要
  数学一般で使用する、定数、構造体、メソッドを集めたシングルトン。
  構造体および、定数と一部のメソッドはstaticなので、インクルードするだけで利用可能。
  解放は不要（アプリケーション終了時に自動）
  座標は、OpenSiv3DのVec2の使用を前提（もし、kotsubu_vecのVEC2を使う場合は、この
  ファイル冒頭、またはこのファイルをインクルードする前に、"USE_KOTSUBU_VEC"をdefineしておく）
  その他、一般的な図形の構造体、テーブル引き三角関数、衝突判定、直角三角形の要素を求める、など

・使い方
  #include "kotsubu_math.h"
  // 定数はインクルードするだけで利用可能
  n = KotsubuMath::Pi;
  // インスタンスを取得。これで全てのメンバにアクセス可能
  KotsubuMath &math = KotsubuMath::getInstance();
  // 一般的な数学メソッド
  n = math.direction(v);
  // 衝突判定（.hitは内部クラスで実装）
  if (math.hit.lineOnline(lineA, lineB)) {
**************************************************************************************************/

#pragma once

//#define USE_KOTSUBU_VEC  // kotsubu_vecのVEC2を利用するなら定義

#include <vector>
#include <cmath>
#ifdef USE_KOTSUBU_VEC
    #include "kotsubu_vec.h"
#else
    #include <Siv3D.hpp>
#endif





///////////////////////////////////////////////////////////////////////////////////////////////
// 【クラス】KotsubuMath
//
class KotsubuMath
{
#ifdef USE_KOTSUBU_VEC
    using Vec2 = VEC2<double>;  // kotsubu_vecのVEC2を利用する
#endif
    


public:
    // 【構造体】図形定義用
    struct Line
    {
        Vec2 startPos, endPos;
        Line() : startPos(Vec2(0.0, 0.0)), endPos(Vec2(0.0, 0.0)) {}
        Line(Vec2 startPos, Vec2 endPos) : startPos(startPos), endPos(endPos) {}
    };

    struct Rect
    {
        double left, top, right, bottom;
        Rect() : left(0.0), top(0.0), right(0.0), bottom(0.0) {}
        Rect(double left, double top, double right, double bottom) : left(left), top(top), right(right), bottom(bottom) {}
    };

    struct Circle
    {
        Vec2 pos;
        double radius;
        Circle() : pos(Vec2(0.0, 0.0)), radius(0.0) {}
        Circle(Vec2 pos, double radius) : pos(pos), radius(radius) {}
    };



    // 【定数】数学一般
    static constexpr double Epsilon    = 0.00001;           // これ未満を0とする
    static constexpr double Pi         = 3.141592653589793; // π
    static constexpr double TwoPi      = Pi * 2.0;          // Radianの最大値
    static constexpr double RightAngle = Pi / 2.0;          // 直角（90°）のRadian
    static constexpr double Deg2Rad    = Pi / 180.0;        // Degに掛けるとRad
    static constexpr double Rad2Deg    = 180.0 / Pi;        // Radに掛けるとDeg
    static constexpr double RootTwo    = 1.414213562373095; // 斜辺が45°の直角三角形における、斜辺の比（他の辺は共に1）
    static constexpr double RoundFix   = 0.5;               // これを正の小数に足して整数にすると四捨五入
    static constexpr double One        = 1.0;               // double 1.0
    static constexpr double Two        = 2.0;               // double 2.0
    static constexpr double Half       = 0.5;               // double 0.5



    // 【メソッド】唯一のインスタンスの参照を返す
    // なるべく計算のロジックに近い所で受け取るようにすると、キャッシュに乗るためか高速化する。
    // 初回時のみ、インスタンスの生成と、数学用テーブルの作成が行われる
    static KotsubuMath& getInstance()
    {
        static KotsubuMath inst;
        return inst;
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 数学メソッド

    // 【メソッド】sin（テーブル引き）
    // radianに「1周 + 30°」を指定した場合は、周を省いた「30°」で計算する（負数も同様）
    double sin(double radian)
    {
        int id = std::abs(static_cast<int>(radian * Sin.Resolution)) % Sin.ScaledTwoPi;

        if (id < Sin.TableMax)
            return (radian < 0.0) ? -Sin.table[id] :  Sin.table[id];
        else {
            id -= Sin.TableMax;
            return (radian < 0.0) ?  Sin.table[id] : -Sin.table[id];
        }
    }



    // 【メソッド】cos（テーブル引き）
    // radianに「1周 + 30°」を指定した場合は、周を省いた「30°」で計算する（負数も同様）
    double cos(double radian)
    {
        return sin(radian + RightAngle);
    }



    // 【メソッド】asin（テーブル引き）
    // ＜戻り値＞ 
    // ratio >  1  ---  ratioが 1のときの値を返す
    // ratio < -1  ---  ratioが-1のときの値を返す
    // 上記は<cmath>の場合、NaNを返す
    double asin(double ratio)
    {
        int id = std::abs(static_cast<int>(ratio * ratio * Asin.TableMax + RoundFix));
        if (id >= Asin.TableMax) id = Asin.TableMax - 1;

        return (ratio < 0.0) ? -Asin.table[id] : Asin.table[id];
    }



    // 【メソッド】acos（テーブル引き）
    // ＜戻り値＞ 
    // ratio >  1  ---  ratioが 1のときの値を返す
    // ratio < -1  ---  ratioが-1のときの値を返す
    // 上記は<cmath>の場合、NaNを返す
    double acos(double ratio)
    {
        return RightAngle - asin(ratio);
    }



    // 【メソッド】ベクトルの長さを返す
    static double length(Vec2 v)
    {
        return sqrt(lengthPow(v));
    }



    // 【メソッド】ベクトルの長さを返す（ルートを取らない）
    static double lengthPow(Vec2 v)
    {
        return v.x * v.x + v.y * v.y;
    }



    // 【メソッド】2点間の距離を返す
    static double distance(Vec2 a, Vec2 b)
    {
        return sqrt(distancePow(a, b));
    }



    // 【メソッド】2点間の距離を返す（ルートを取らない）
    static double distancePow(Vec2 a, Vec2 b)
    {
        Vec2 v(a - b);
        return v.x * v.x + v.y * v.y;
    }



    // 【メソッド】ベクトルを正規化して返す
    static Vec2 normalize(Vec2 v)
    {
        double len = length(v);
        if (len < Epsilon) return v;

        return v *= inverseNumber(len);
    }



    // 【メソッド】内積を返す
    static double innerProduct(Vec2 a, Vec2 b)
    {
        return a.x * b.x + a.y * b.y;
    }



    // 【メソッド】内積を返す（aとスクリーンx）
    static double innerProduct(Vec2 a)
    {
        // 基準の軸は、成分x=1,y=0なので、数式がとても簡単
        // return a.x * 1.0 + a.y * 0.0;
        return a.x;
    }



    // 【メソッド】外積を返す
    static double outerProduct(Vec2 a, Vec2 b)
    {
        return a.x * b.y - b.x * a.y;
    }



    // 【メソッド】外積を返す（aとスクリーンx）
    static double outerProduct(Vec2 a)
    {
        // 基準の軸は、成分x=1,y=0なので、数式がとても簡単
        // return a.x * 0.0 - 1.0 * a.y;
        // return 0.0 - a.y;
        return -a.y;
    }



    // 【メソッド】ベクトルの向きを返す（スクリーン座標系。atan2の代わりに使えて高速）
    // ＜戻り値＞ -180°から180°のradian
    // 例外以外は、std::atan2に準拠
    double direction(double vx, double vy)
    {
        double len = sqrt(vx * vx + vy * vy);
        if (len < Epsilon) return 0.0;

        // 基準の軸は、成分x=1,y=0なので、数式がとても簡単
        double cosVal = vx / len;  // 「内積/長さ」で余弦値を求める
        return (vy < 0.0) ? -acos(cosVal) : acos(cosVal);  // 外積を見て、360度角を得る
    }

    double direction(Vec2 v)
    {
        return direction(v.x, v.y);
    }



    // 【メソッド】ベクトルaから見た「bの方角」を返す
    // ＜戻り値＞ -180°から180°のradian（時計回りを正）
    // aから見たbの方角は、+と-の2通りが存在する。このメソッドでは近い方（±180°を超えない方）を返す。
    // よって、戻り値の符号だけ（-0.1とか0.1）をaに足し込むと、段々bの方に向かせることができる。
    // また、bがaの「左右どちらにあるか」の判定にも使えるが、外積のほうが高速。
    // ・戻り値の範囲違いの類似処理
    // direction(b) - direction(a)  ---  -360°から360°（高速。-10°の方が近くても350°になったりする）
    // fmod(direction(b) - direction(a) + TwoPi, TwoPi)  ---  0°から360°
    double angle(Vec2 a, Vec2 b)
    {
        double rad = direction(b) - direction(a);
        if (rad > Pi)
            rad -= TwoPi;  //  200°等であれば-160°とする
        else if (rad < -Pi)
            rad += TwoPi;  // -200°等であれば 160°とする

        return rad;
    }



    // 【メソッド】ベクトルを回転して返す
    static Vec2 rotation(Vec2 v, double sinVal, double cosVal)
    {
        return { v.x * cosVal - v.y * sinVal,
                 v.x * sinVal + v.y * cosVal };
    }

    Vec2 rotation(Vec2 v, double radian)
    {
        return rotation(v, sin(radian), cos(radian));
    }



    // 【メソッド】反射角を返す
    // ＜引数＞incidenceRadは入射角、reflectionAxisRadは壁となる軸の角度
    static double reflection(double incidenceRad, double reflectionAxisRad)
    {
        // 式。「壁となる軸の角度 * 2」から入射角を引く
        return fmod(reflectionAxisRad * Two - incidenceRad, TwoPi);
    }



    // 【メソッド】逆数を返す
    // 「割る数」を「掛ける数」に変換。またはその逆
    static double inverseNumber(double num)
    {
        return One / num;
    }



    // 【メソッド】度数をラジアンに変換
    // 0～2πの範囲に調整する。通常は「degree * KotsubuMath::Deg2Rad」でよい
    static double toRadian(double degree)
    {
        if (degree < 0.0) {
            degree = fmod(degree, 360.0) + 360.0;
            if (degree == 360.0) degree = 0.0;
        }
        else if (degree >= 360.0)
            degree = fmod(degree, 360.0);

        return degree * Deg2Rad;
    }



    // 【メソッド】度数の角度範囲をラジアンに変換
    // 0°未満は0、360°より大きいなら2πに制限する
    static double toRadianRange(double degreeRange)
    {
        if (degreeRange <   0.0) degreeRange = 0.0;
        if (degreeRange > 360.0) degreeRange = 360.0;

        return degreeRange * Deg2Rad;
    }



    // 【メソッド】割った余りを返す（std::fmodより高速）
    static double fmod(double num, double divNum)
    {
        return num - divNum * static_cast<int>(num / divNum);
    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 【内部クラス】直角三角形の性質
    // 直角三角形の定義  ---  ⊿abc, 頂点は上から反時計回りにa,b,cとする。斜辺はab、頂点cは直角
    // このクラスは、実用性よりも学習、コピペ向けに実装
    //
    class RightTriangle
    {
    public:
        // 【メソッド】斜辺の長さを返す（三平方の定理）
        // ＜引数＞ 底辺の長さ、高さ
        static double hypotLen(double baseLen, double height)
        {
            return sqrt(baseLen * baseLen + height * height);
        }



        // 【メソッド】底辺の長さを返す（内積）
        // 斜辺abと長さ不定の底辺bcから直角三角形を定義して、底辺の長さを算出する
        // ＜引数＞ 斜辺ab、底辺bcの座標（底辺の長さは適当でよい）
        // 頂点abcの関係が「鈍角（90°以上）」のときは、通常の直角三角形は定義できない。
        // その場合は、直線bc上に「反転された直角三角形」を形成して結果を求める。
        static double baseLen(Vec2 a, Vec2 b, Vec2 c)
        {
            // ふつうの三角形を定義
            Vec2 abV(a - b);             // 斜辺ベクトル
            Vec2 bcV(c - b);             // 底辺ベクトル
            double bcLen = length(bcV);  // 底辺の長さ（まだ直角三角形にしたときの底辺は不明）
            if (bcLen < Epsilon) return 0.0;

            // 直角三角形の底辺長 = abとbcの内積を、bc長で割る。
            // これは、斜辺を線分に正投影したときの「影の長さ」に相当。
            // もし、影が逆方向（線分始点より手前。鈍角）なら負の数になる
            return innerProduct(abV, bcV) / bcLen;
        }



        // 【メソッド】底辺の長さを返す（内積）
        // 斜辺abと傾き（斜辺から見た底辺の方角）から直角三角形を定義して、底辺の長さを算出する
        // ＜引数＞ 斜辺abの座標、傾き（radian）
        // 傾きは、通常±90°未満を指定する。正で反時計回り、負で時計回りの図形となる。
        // また、±90°を超えると「高さの辺」が斜辺をまたいで図形が反転する。
        double baseLen(Vec2 a, Vec2 b, double bAngle)
        {
            KotsubuMath& math = getInstance();  // 親クラスの静的ではないメソッドを利用する
            Vec2   abV(a - b);                                  // 斜辺ベクトル
            double abDir = math.direction(abV);                 // 斜辺の傾き
            double bcDir = abDir + bAngle;                      // 底辺の傾き
            Vec2   bcNormal(math.cos(bcDir), math.sin(bcDir));  // 底辺の正規化ベクトル

            // 直角三角形の底辺長 = abと正規化bcの内積。
            // これは、斜辺を線分bcに正投影したときの「影の長さ」に相当。
            // もし、影が逆方向（線分始点より手前。鈍角）なら負の数になる
            return innerProduct(abV, bcNormal);
        }



        // 【メソッド】高さを返す（外積）
        // 斜辺abと長さ不定の底辺bcから直角三角形を定義して、高さを算出する
        // ＜引数＞ 斜辺ab、底辺bcの座標（底辺の長さは適当でよい）
        // 頂点abcの関係が「鈍角（90°以上）」のときは、通常の直角三角形は定義できない。
        // その場合は、直線bc上に「反転された直角三角形」を形成して結果を求める。
        static double height(Vec2 a, Vec2 b, Vec2 c)
        {
            // ふつうの三角形を定義
            Vec2 abV(a - b);             // 斜辺ベクトル
            Vec2 bcV(c - b);             // 底辺ベクトル
            double bcLen = length(bcV);  // 底辺の長さ
            if (bcLen < Epsilon) return 0.0;

            // 直角三角形の高さ = abとbcの外積を、bc長で割る。
            // これは、点aを通る線分bcの「垂線」に相当。
            // absは、点が線分の「左右どちらにあるかで符号が変わる」ため
            return std::abs(outerProduct(abV, bcV) / bcLen);
        }
        


        // 【メソッド】高さを返す（外積）
        // 斜辺abと傾き（斜辺から見た底辺の方角）から直角三角形を定義して、高さを算出する
        // ＜引数＞ 斜辺abの座標、傾き（radian）
        // 傾きは、通常±90°未満を指定する。正で反時計回り、負で時計回りの図形となる。
        // また、±90°を超えると「高さの辺」が斜辺をまたいで図形が反転する。
        double height(Vec2 a, Vec2 b, double bAngle)
        {
            KotsubuMath& math = getInstance();  // 親クラスの静的ではないメソッドを利用する
            Vec2   abV(a - b);                                  // 斜辺ベクトル
            double abDir = math.direction(abV);                 // 斜辺の傾き
            double bcDir = abDir + bAngle;                      // 底辺の傾き
            Vec2   bcNormal(math.cos(bcDir), math.sin(bcDir));  // 底辺の正規化ベクトル

            // 直角三角形の高さ = abと正規化bcの外積。
            // これは、点aを通る線分bcの「垂線」に相当。
            // absは、点が線分の「左右どちらにあるかで符号が変わる」ため
            return std::abs(outerProduct(abV, bcNormal));
        }



        // 【メソッド】底辺終点の座標を返す（直角三角形の頂点c）
        // 斜辺abと長さ不定の底辺bcから直角三角形を定義して、底辺終点の座標を算出する。
        // これは、斜辺を地面に正投影したときの「影の終わりの位置」に相当する。
        // ＜引数＞ 斜辺ab、底辺bcの座標（底辺の長さは適当でよい）
        // 頂点abcの関係が「鈍角（90°以上）」のときは、通常の直角三角形は定義できない。
        // その場合は、直線bc上に「反転された直角三角形」を形成して結果を求める。
        static Vec2 baseEndPos(Vec2 a, Vec2 b, Vec2 c)
        {
            // ふつうの三角形を定義
            Vec2 abV(a - b);             // 斜辺ベクトル
            Vec2 bcV(c - b);             // 底辺ベクトル
            double bcLen = length(bcV);  // 底辺の長さ（まだ直角三角形にしたときの底辺は不明）
            if (bcLen < Epsilon) return b;  // 底辺の長さが0の場合は「頂点b = 底辺終点」となる

            // 底辺終点 = 底辺始点 + 底辺ベクトル * その割合
            return b + bcV * innerProduct(abV, bcV) / (bcLen * bcLen);
        }
        


        // 【メソッド】底辺終点の座標を返す（直角三角形の頂点c）
        // 斜辺abと傾き（斜辺から見た底辺の方角）から直角三角形を定義して、底辺終点の座標を算出する。
        // これは、斜辺を地面に正投影したときの「影の終わりの位置」に相当する。
        // ＜引数＞ 斜辺abの座標、傾き（radian）
        // 傾きは、通常±90°未満を指定する。正で反時計回り、負で時計回りの図形となる。
        // また、±90°を超えると「高さの辺」が斜辺をまたいで図形が反転する。
        Vec2 baseEndPos(Vec2 a, Vec2 b, double bAngle)
        {
            KotsubuMath& math = getInstance();  // 親クラスの静的ではないメソッドを利用する
            Vec2   abV(a - b);                                  // 斜辺ベクトル
            double abDir = math.direction(abV);                 // 斜辺の傾き
            double bcDir = abDir + bAngle;                      // 底辺の傾き
            Vec2   bcNormal(math.cos(bcDir), math.sin(bcDir));  // 底辺の正規化ベクトル

            // 底辺終点 = 底辺始点 + 底辺の正規化ベクトル * その長さ
            return b + bcNormal * innerProduct(abV, bcNormal);
        }


 
        // 【メソッド】斜辺と底辺のなす角（∠b）を返す
        // ＜引数＞ 斜辺ab、底辺bcの座標（それぞれ長さは適当でよい）
        // ＜戻り値＞ ±180°以下の数。図形が反時計回りのときは正、時計回りのときは負
        double angleB(Vec2 a, Vec2 b, Vec2 c)
        {
            KotsubuMath& math = getInstance();  // 親クラスの静的ではないメソッドを利用する
            Vec2 abV(a - b);  // 斜辺ベクトル
            Vec2 bcV(c - b);  // 底辺ベクトル

            return math.angle(abV, bcV);
        }

    } rightTriangle;





    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 【内部クラス】点と線分の性質
    // このクラスは、実用性よりも学習、コピペ向けに実装
    //
    class PointAndLine
    {
    public:
        // 【メソッド】最短距離を返す（点と線分）
        // まず、点の線分に対する「垂線」を考える。
        // 交点が線分上にあるなら、垂線の長さが「最短距離」となる。
        // 線分上に無いなら、近いほうの線分端までが「最短距離」となる。
        // これは、戻り値が「ある半径以内」かどうかを見て「円と線分の衝突判定」に利用できる
        static double distance(Vec2 point, Line line)
        {
            Vec2   lineV(line.endPos - line.startPos);
            double lineLen = length(lineV);
            // 線分が短すぎる場合は、正常な計算ができない。点⇔線分始点の距離を返して終了
            if (lineLen < Epsilon)
                return KotsubuMath::distance(point, line.startPos);

            // 点⇔線分始点を結ぶ辺が「鈍角」なら、始点が最も近い
            if (innerProduct(point - line.startPos, lineV) < 0.0)
                return KotsubuMath::distance(point, line.startPos);

            // 点⇔線分終点を結ぶ辺が「鋭角」なら、終点が最も近い
            if (innerProduct(point - line.endPos, lineV) >= 0.0)
                return KotsubuMath::distance(point, line.endPos);
            
            // 上記以外（交点が線分上にある）なら、垂線の長さが最短距離
            return std::abs(outerProduct(point - line.startPos, lineV)) / lineLen;
        }



        // 【メソッド】垂線の交点を返す（点と線分）
        static Vec2 intersectPos(Vec2 point, Line line)
        {
            // 直角三角形に置きかえて計算
            // 垂線の交点 = 線分始点 + 点と線分の内積（影の長さ）
            return RightTriangle::baseEndPos(point, line.startPos, line.endPos);
        }

    } pointAndLine;





    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 【内部クラス】衝突判定
    //
    class HitTest
    {
    public:
        // 【メソッド】交差判定。線分と線分
        // ＜引数＞
        // posA --- 線分1の始点
        // posB --- 線分1の終点
        // posC --- 線分2の始点
        // posD --- 線分2の終点
        static bool lineOnLine(Vec2 posA, Vec2 posB, Vec2 posC, Vec2 posD)
        {
            Vec2 vecAB(posB - posA);
            Vec2 vecCD(posD - posC);
            Vec2 vecAC(posC - posA);
            Vec2 vecAD(posD - posA);
            Vec2 vecCA(posA - posC);
            Vec2 vecCB(posB - posC);
            
            // 線分と線分の交差判定の方法（他にも様々な方法がある）
            // 1. 直線ABからみて、線分CDの頂点Cが左側にあり、かつ頂点Dが右側にある。
            // 2. 直線CDからみて、線分ABの頂点Aが左側にあり、かつ頂点Bが右側にある。
            // 上記1と2を満たすとき、交差している。
            // 「直線ABと頂点Cの外積 * 直線ABと頂点Dの外積」で、直線ABに線分CDがまたいでいるかが分かる。
            // 「正*正>0, 正*負<0, 負*負>0」の性質を利用している（左側と右側が入れ替わっても同じ）
            return (outerProduct(vecAB, vecAC) * outerProduct(vecAB, vecAD) < 0.0) &&
                   (outerProduct(vecCD, vecCA) * outerProduct(vecCD, vecCB) < 0.0);
        }

        static bool lineOnLine(Line lineA, Line lineB)
        {
            return lineOnLine(lineA.startPos, lineA.endPos, lineB.startPos, lineB.endPos);
        }



        // 【メソッド】交差判定。線分とスクリーン横軸
        // ＜引数＞
        // lineStartY  --- 線分の始点y
        // lineEndY    --- 線分の終点y
        // horizontalY --- 横軸のy座標
        static bool lineOnHorizontal(double lineStartY, double lineEndY, double horizontalY)
        {
            double a = horizontalY - lineStartY;
            double b = horizontalY - lineEndY;
            return a * b < 0.0;
        }



        // 【メソッド】交差判定。線分とスクリーン縦軸
        // ＜引数＞
        // lineStartX --- 線分の始点x
        // lineEndX   --- 線分の終点x
        // verticalX  --- 縦軸のx座標
        static bool lineOnVertical(double lineStartX, double lineEndX, double verticalX)
        {
            double a = verticalX - lineStartX;
            double b = verticalX - lineEndX;
            return a * b < 0.0;
        }



        // 【メソッド】内包判定。点と矩形
        // ＜引数＞
        // point --- 点の座標
        // boxLeft, boxTop, boxRight, boxBottom --- 矩形の座標
        static bool pointOnBox(Vec2 point, double boxLeft, double boxTop, double boxRight, double boxBottom)
        {
            return (point.x >= boxLeft) && (point.y >= boxTop) &&
                   (point.x < boxRight) && (point.y < boxBottom);
        }

        static bool pointOnBox(Vec2 point, Rect box)
        {
            return pointOnBox(point, box.left, box.top, box.right, box.bottom);
        }



        // 【メソッド】内包判定。点と多角形（すべての辺の内側かどうか）
        // ＜引数＞
        // point    --- 点の座標
        // vertices --- 多角形を構成する頂点。vector<Vec2>
        // ＜正しい結果を得るには＞
        // 図形の頂点は右回り（左回りなら結果は逆となる）、閉じた図形、全ての内角は180°以下。
        // 上記を満たさない場合はエラーにならず、不定な動作となる
        static bool pointOnPolygon(Vec2 point, const std::vector<Vec2>& vertices)
        {
            // 頂点nと頂点n+1を結ぶ辺から見て、点が「左側」にあった時点で判定をやめる
            for (int i = 0, edgeQty = vertices.size() - 1; i < edgeQty; ++i) {
                Line edge(vertices[i], vertices[i + 1]);
                if (outerProduct(edge.endPos - edge.startPos, point - edge.startPos) < 0.0)
                    return false;
            }
            return true;
        }

    } hit;





private:
    // 【構造体】テーブル用
    struct SinTable
    {
        static constexpr int Resolution  = 2000;
        static constexpr int TableMax    = static_cast<int>(Pi * Resolution);
        static constexpr int ScaledTwoPi = static_cast<int>(TwoPi * Resolution);
        double table[TableMax];
    } Sin;

    struct AsinTable
    {
        static constexpr int Resolution  = 3000;
        static constexpr int TableMax    = Resolution;
        double table[TableMax];
    } Asin;



    // 【隠しメソッド】
    // 隠しコンストラクタ
    KotsubuMath()
    {
        // sinテーブルを作成（cos兼用）
        double n;
        for (int i = 0; i < Sin.TableMax; ++i) {
            n = std::sin(static_cast<double>(i) / Sin.Resolution);
            Sin.table[i] = (n < Epsilon) ? 0.0 : n;
        }
        
        // asinテーブルを作成（acos兼用）
        double max = Asin.TableMax - 1;
        for (int i = 0; i < Asin.TableMax; ++i) {
            n = std::asin(sqrt(i / max));
            Asin.table[i] = (n < Epsilon) ? 0.0 : n;
        }
    }

    ~KotsubuMath(){}                             // 隠しデストラクタ
    KotsubuMath(const KotsubuMath&);             // 隠しコピーコンストラクタ
    KotsubuMath& operator=(const KotsubuMath&);  // 隠しコピー代入演算子
};

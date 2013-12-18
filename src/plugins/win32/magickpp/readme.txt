Title: Magick++ Plugin
Author: Miahmie.

※【注意】一応動く状態ではありますが，まだ未完成です！ 
  ⇒ImageMagick/Magick++のすべての機能が使用できるわけではありません


● なにか？

ImageMagick の Magick++ を TJS で使えるようになるかもしれません。
とりあえず今必要な機能はだいたい揃ってしまったので，
今後，描画などの機能を全部実装するかどうか微妙なところです。


● 使用方法

MagickPP
MagickPP.クラス名 (MagickPP.Image, MagickPP.Geometry, MagickPP.Color, etc.)
MagickPP.Enum型.Enum値 (MagickPP.ColorspaceType.RGBColorspace, etc.)

あたりが拡張されます。※【注意】名前は将来変更される可能性があります。

メソッド名やプロパティはほぼ Magick++ 準拠ですが言語仕様により
別名が割り振られている場合があります。

enum 値はそれ専用の型を持っているわけではなく，
intにキャストされた値を返します。


現状では詳しくはソース見てください状態です。（すみません）


あと，問題点として，
重い処理中（PSDの読み込み等）は完全に吉里吉里の反応が止まります。
シングルスレッド仕様なので現状で回避方法はありません。

重い処理中のメッセージや，ウィンドウなどを表示したいときは，
System.inform で処理前にクリック待ちを促すか，
AsyncTrigger を使用して，一度メッセージ処理を完遂してから
トリガファンクション内で重い処理を行うと良いでしょう。

	// これだとコンソールにメッセージが表示される前に
	// ロード処理で固まってしまい，メッセージが見えない
	Debug.message("ロード中です");
	image.read(filename);

	// クリック待ちがあっても良いなら
	// モーダルダイアログで注意を促す
	System.inform("ロードします" + filename);
	image.read(filename);

	// クリック待ちなしで単に表示したいならこんな感じ
	Debug.message("ロード中です");
	(new AsyncTrigger(function() {
	  image.read(filename);
	  load_finished(image);
	}, "")).trigger();
	// ただしここの行を処理する段階ではまだ読み込みが
	// 完了してないので，トリガファンクションに
	// すべての処理を記述する必要がある
	return;


● 実装状況

元のライブラリが巨大なので ncbind をもってしても
メソッドの口を作るだけでも大変。
また実装されている機能を全部テストしているわけではありません。

・Geometry
・Color
	ほぼ完了
	文字列変換は string プロパティを読み書きすることで行う。
		例：
			myColor.string = "#102030";
			Debug.message(myGeometry.string);

・Image
	ほぼ機能は揃ってるが，オーバーロードされているメソッドや
	配列を渡すようなメソッドは実装されていない。

	値を複数渡すようなプロパティはメソッドとして実装されている
	メモ：
	attribute   ⇒ getAttribute(str), setAttribute(str, str)
	colorMap    ⇒ getColorMap(int),  setColorMap(int, color)
	defineValue ⇒ getDefineValue(str, str), setDefineValue(str, str, str)
	defineSet   ⇒ getDefineValue(str, str), setDefineValue(str, str, bool)

	gamma ⇒ doubleの読み書きプロパティ と setGamma(r,g,b) メソッド

	fontTypeMetrics ⇒ method で実装
			   TypeMetric imageInstance.fontTypeMetrics(str);

	fillRule ⇒ ncbind の不具合で保留

	chroma{Red,Blue,Green}Primary
	chromaWhitePoint
			⇒値を複数返すので保留

	strokeDashArray		⇒ double * を渡すので保留
	convolve		⇒ double * を渡すので保留


	display() メソッドは display(layer) メソッドに変更
	表示ではなくレイヤにイメージをコピーする処理にしてある


・CoderInfo
	完了。MagickPP.support で使用

・その他
	インスタンスのラッパは存在するがメソッドの口は用意されていない
	enum 値はヘッダに用意されているものはすべて登録したつもり
	(Magick++で使用されないPreviewTypeを除く)

・MagickPP
	STL関連の単体メソッドや enum などを集約する予定
	すべて static なメソッド/プロパティで，インスタンスは生成不可

	現状では
		version プロパティ ⇒ バージョン文字列を返す
		support プロパティ ⇒ サポートされるCoderInfoの配列を返す

		readImages(string) ⇒ Magick::readImages
					読み込んだ Image の配列を返す

	が存在する。


● コンパイル

MSYS/MinGW でコンパイルを確認してあります。
あらかじめ static link archive な ImageMagick を
コンパイル・インストールしておいた状態で make すれば OK です。

make test によるテスト環境は svn 上のツリー形状と同じ環境での
実行を前提としています。多分。


● ライセンス

ImageMagick は GPL 互換ラインセンスです。多分。
詳しくは http://www.imagemagick.org/script/license.php をご覧ください。

------------------------------------------------------------------------------
Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization dedicated to making software imaging solutions freely available.

1. Definitions.

"License" shall mean the terms and conditions for use, reproduction, and distribution as defined by Sections 1 through 9 of this document.
  
"Licensor" shall mean the copyright owner or entity authorized by the copyright owner that is granting the License.
  
"Legal Entity" shall mean the union of the acting entity and all other entities that control, are controlled by, or are under common control with that entity. For the purposes of this definition, "control" means (i) the power, direct or indirect, to cause the direction or management of such entity, whether by contract or otherwise, or (ii) ownership of fifty percent (50%) or more of the outstanding shares, or (iii) beneficial ownership of such entity.
  
"You" (or "Your") shall mean an individual or Legal Entity exercising permissions granted by this License.
  
"Source" form shall mean the preferred form for making modifications, including but not limited to software source code, documentation source, and configuration files.
  
"Object" form shall mean any form resulting from mechanical transformation or translation of a Source form, including but not limited to compiled object code, generated documentation, and conversions to other media types.
  
"Work" shall mean the work of authorship, whether in Source or Object form, made available under the License, as indicated by a copyright notice that is included in or attached to the work (an example is provided in the Appendix below).
  
"Derivative Works" shall mean any work, whether in Source or Object form, that is based on (or derived from) the Work and for which the editorial revisions, annotations, elaborations, or other modifications represent, as a whole, an original work of authorship. For the purposes of this License, Derivative Works shall not include works that remain separable from, or merely link (or bind by name) to the interfaces of, the Work and Derivative Works thereof.
  
"Contribution" shall mean any work of authorship, including the original version of the Work and any modifications or additions to that Work or Derivative Works thereof, that is intentionally submitted to Licensor for inclusion in the Work by the copyright owner or by an individual or Legal Entity authorized to submit on behalf of the copyright owner. For the purposes of this definition, "submitted" means any form of electronic, verbal, or written communication intentionally sent to the Licensor by its copyright holder or its representatives, including but not limited to communication on electronic mailing lists, source code control systems, and issue tracking systems that are managed by, or on behalf of, the Licensor for the purpose of discussing and improving the Work, but excluding communication that is conspicuously marked or otherwise designated in writing by the copyright owner as "Not a Contribution."
  
"Contributor" shall mean Licensor and any individual or Legal Entity on behalf of whom a Contribution has been received by Licensor and subsequently incorporated within the Work.

2. Grant of Copyright License. Subject to the terms and conditions of this License, each Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable copyright license to reproduce, prepare Derivative Works of, publicly display, publicly perform, sublicense, and distribute the Work and such Derivative Works in Source or Object form.

3. Grant of Patent License. Subject to the terms and conditions of this License, each Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable patent license to make, have made, use, offer to sell, sell, import, and otherwise transfer the Work, where such license applies only to those patent claims licensable by such Contributor that are necessarily infringed by their Contribution(s) alone or by combination of their Contribution(s) with the Work to which such Contribution(s) was submitted.

4. Redistribution. You may reproduce and distribute copies of the Work or Derivative Works thereof in any medium, with or without modifications, and in Source or Object form, provided that You meet the following conditions:

     a. You must give any other recipients of the Work or Derivative Works a copy of this License; and

     b. You must cause any modified files to carry prominent notices stating that You changed the files; and

     c. You must retain, in the Source form of any Derivative Works that You distribute, all copyright, patent, trademark, and attribution notices from the Source form of the Work, excluding those notices that do not pertain to any part of the Derivative Works; and

     d. If the Work includes a "NOTICE" text file as part of its distribution, then any Derivative Works that You distribute must include a readable copy of the attribution notices contained within such NOTICE file, excluding those notices that do not pertain to any part of the Derivative Works, in at least one of the following places: within a NOTICE text file distributed as part of the Derivative Works; within the Source form or documentation, if provided along with the Derivative Works; or, within a display generated by the Derivative Works, if and wherever such third-party notices normally appear. The contents of the NOTICE file are for informational purposes only and do not modify the License. You may add Your own attribution notices within Derivative Works that You distribute, alongside or as an addendum to the NOTICE text from the Work, provided that such additional attribution notices cannot be construed as modifying the License.

You may add Your own copyright statement to Your modifications and may provide additional or different license terms and conditions for use, reproduction, or distribution of Your modifications, or for any such Derivative Works as a whole, provided Your use, reproduction, and distribution of the Work otherwise complies with the conditions stated in this License.

5. Submission of Contributions. Unless You explicitly state otherwise, any Contribution intentionally submitted for inclusion in the Work by You to the Licensor shall be under the terms and conditions of this License, without any additional terms or conditions. Notwithstanding the above, nothing herein shall supersede or modify the terms of any separate license agreement you may have executed with Licensor regarding such Contributions.

6. Trademarks. This License does not grant permission to use the trade names, trademarks, service marks, or product names of the Licensor, except as required for reasonable and customary use in describing the origin of the Work and reproducing the content of the NOTICE file.

7. Disclaimer of Warranty. Unless required by applicable law or agreed to in writing, Licensor provides the Work (and each Contributor provides its Contributions) on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including, without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the Work and assume any risks associated with Your exercise of permissions under this License.

8. Limitation of Liability. In no event and under no legal theory, whether in tort (including negligence), contract, or otherwise, unless required by applicable law (such as deliberate and grossly negligent acts) or agreed to in writing, shall any Contributor be liable to You for damages, including any direct, indirect, special, incidental, or consequential damages of any character arising as a result of this License or out of the use or inability to use the Work (including but not limited to damages for loss of goodwill, work stoppage, computer failure or malfunction, or any and all other commercial damages or losses), even if such Contributor has been advised of the possibility of such damages.

9. Accepting Warranty or Additional Liability. While redistributing the Work or Derivative Works thereof, You may choose to offer, and charge a fee for, acceptance of support, warranty, indemnity, or other liability obligations and/or rights consistent with this License. However, in accepting such obligations, You may act only on Your own behalf and on Your sole responsibility, not on behalf of any other Contributor, and only if You agree to indemnify, defend, and hold each Contributor harmless for any liability incurred by, or claims asserted against, such Contributor by reason of your accepting any such warranty or additional liability.
------------------------------------------------------------------------------

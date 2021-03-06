/*
 * DIPlib 3.0
 * This file contains definitions of threshold estimation algorithms.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diplib.h"
#include "diplib/histogram.h"
#include "diplib/segmentation.h"
#include "diplib/statistics.h"
#include "diplib/chain_code.h" // for VertexFloat, TriangleHeight, etc.


namespace dip {

using CountType = Histogram::CountType;

Histogram KMeansClustering(
      Histogram const& in,
      dip::uint nClusters
) {
   Image labs;
   KMeansClustering( in.GetImage(), labs, nClusters );
   Histogram out = in.Copy();
   Image tmp = out.GetImage(); // Copy with shared data
   tmp.Copy( labs );
   return out;
}

Histogram MinimumVariancePartitioning(
      Histogram const& in,
      dip::uint nClusters
) {
   Image labs;
   MinimumVariancePartitioning( in.GetImage(), labs, nClusters );
   Histogram out = in.Copy();
   Image tmp = out.GetImage(); // Copy with shared data
   tmp.Copy( labs );
   return out;
}

FloatArray IsodataThreshold(
      Histogram const& in,
      dip::uint nThresholds
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   FloatArray thresholds( nThresholds );
   // Initialize thresholds such that each interval has approximately same number of pixels
   Histogram cumh = CumulativeHistogram( in );
   Image const& cum = cumh.GetImage();
   DIP_ASSERT( cum.IsForged() );
   DIP_ASSERT( cum.DataType() == DT_COUNT );
   DIP_ASSERT( cum.Stride( 0 ) == 1 );
   DIP_ASSERT( cum.Size( 0 ) == nBins );
   Histogram::CountType* ptr = static_cast< Histogram::CountType* >( cum.Origin() );
   dip::uint N = ptr[ nBins - 1 ] / ( nThresholds + 1 );
   dip::uint index = 1;
   for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
      while( ptr[ index ] < N * ( ii + 1 )) {
         ++index;
      }
      thresholds[ ii ] = static_cast< dfloat >( index );
   }
   // Apply the iterative process
   FloatArray old;
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   do {
      old = thresholds;
      dip::uint origin1 = 0;
      dip::uint origin2;
      FloatArray centers( nThresholds + 1 );
      for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
         origin2 = static_cast< dip::uint >( std::ceil( thresholds[ ii ] ));
         dfloat moment = 0;
         dfloat sum = 0;
         for( dip::uint jj = origin1; jj < origin2; ++jj ) {
            moment += static_cast< dfloat >( jj ) * static_cast< dfloat >( data[ jj ] );
            sum += static_cast< dfloat >( data[ jj ] );
         }
         if( sum > 0 ) {
            centers[ ii ] = moment / sum;
         } else {
            centers[ ii ] = static_cast< dfloat >( origin1 + origin2 ) / 2.0;
         }
         origin1 = origin2;
      }
      dfloat moment = 0;
      dfloat sum = 0;
      for( dip::uint jj = origin1; jj < nBins; ++jj ) {
         moment += static_cast< dfloat >( jj ) * static_cast< dfloat >( data[ jj ] );
         sum += static_cast< dfloat >( data[ jj ] );
      }
      if( sum > 0 ) {
         centers.back() = moment / sum;
      } else {
         centers.back() = static_cast< dfloat >( origin1 + nBins ) / 2.0;
      }
      for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
         thresholds[ ii ] = ( centers[ ii + 1 ] + centers[ ii ] ) / 2.0;
      }
   } while( thresholds != old );
   // Translate thresholds from bin indices to intensities (using linear interpolation)
   dfloat scale = in.BinSize();
   dfloat offset = in.LowerBound() + scale / 2; // bin[ii] = offset + ii * scale;
   for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
      thresholds[ ii ] = offset + thresholds[ ii ] * scale;
   }
   return thresholds;
}

dfloat OtsuThreshold(
      Histogram const& in
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   FloatArray bins = in.BinCenters();
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   dfloat const* binPtr = bins.data();
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between bins(ii) and bins(ii+1)
   dfloat w1 = 0;
   dfloat w2 = std::accumulate( data, data + nBins, 0.0 );
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = std::inner_product( data, data + nBins, binPtr, 0.0 );
   // Here we accumulate the max.
   dfloat ssMax = -1e6;
   dip::uint maxInd = 0;
   for( dip::uint ii = 0; ii < nBins - 1; ++ii ) {
      w1 += static_cast< dfloat >( *data );
      w2 -= static_cast< dfloat >( *data );
      dfloat tmp = static_cast< dfloat >( *data ) * *binPtr;
      m1 += tmp;
      m2 -= tmp;
      // c1(ii), c2(ii) are the centers of gravity
      dfloat c1 = m1 / w1;
      dfloat c2 = m2 / w2;
      dfloat c = c1 - c2;
      // ss(ii) is Otsu's measure for inter-class variance
      dfloat ss = w1 * w2 * c * c;
      if( ss > ssMax ) {
         ssMax = ss;
         maxInd = ii;
      }
      ++data;
      ++binPtr;
   }
   DIP_THROW_IF( ssMax == -1e6, "Could not find a maximum in Otsu's measure for inter-class variance" );
   return ( bins[ maxInd ] + bins[ maxInd + 1 ] ) / 2.0;
}

dfloat MinimumErrorThreshold(
      Histogram const& in
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   dfloat scale = in.BinSize();
   dfloat offset = in.LowerBound() + scale / 2; // bin[ii] = offset + ii * scale;
   Histogram::CountType const* data = static_cast< Histogram::CountType* >( hist.Origin() );
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between bin(ii) and bin(ii+1)
   dfloat w1 = 0;
   dfloat w2 = std::accumulate( data, data + nBins, 0.0 );
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = 0;
   for( dip::uint ii = 0; ii < nBins; ++ii ) {
      m2 += static_cast< dfloat >( data[ ii ] ) * ( offset + static_cast< dfloat >( ii ) * scale );
   }
   // Here we accumulate the error measure.
   std::vector< double > J( nBins - 1 );
   for( dip::uint ii = 0; ii < nBins - 1; ++ii ) {
      dfloat value = static_cast< dfloat >( data[ ii ] );
      w1 += value;
      w2 -= value;
      dfloat tmp = value * ( offset + static_cast< dfloat >( ii ) * scale );
      m1 += tmp;
      m2 -= tmp;
      // c1(ii), c2(ii) are the centers of gravity
      dfloat c1 = m1 / w1;
      dfloat c2 = m2 / w2;
      // v1(ii), v2(ii) are the corresponding second order central moments
      dfloat v1 = 0;
      for( dip::uint jj = 0; jj <= ii; ++jj ) {
         dfloat d = ( offset + static_cast< dfloat >( jj ) * scale ) - c1;
         v1 += static_cast< dfloat >( data[ jj ] ) * d * d;
      }
      v1 /= w1;
      dfloat v2 = 0;
      for( dip::uint jj = ii + 1; jj < nBins; ++jj ) {
         dfloat d = ( offset + static_cast< dfloat >( jj ) * scale ) - c2;
         v2 += static_cast< dfloat >( data[ jj ] ) * d * d;
      }
      v2 /= w2;
      // J(ii) is the measure for error
      J[ ii ] = 1.0 + w1 * std::log( v1 ) + w2 * std::log( v2 ) - 2.0 * ( w1 * std::log( w1 ) + w2 * std::log( w2 ));
      std::cout << J[ii] << ",";
   }
   std::cout << '\n';
   // Now we need to find the minimum in J, but ignore the values at the edges, if they are lowest.
   dip::uint begin = 0;
   dip::uint end = nBins - 2;
   for( ; ( begin < end - 1 ) && ( J[ begin ] <= J[ begin + 1 ] ); ++begin ) {}
   for( ; ( begin < end - 1 ) && ( J[ end ] <= J[ end - 1 ] ); --end ) {}
   dfloat minJ = J[ begin ];
   dip::uint minInd = begin;
   for( dip::uint ii = begin + 1; ii < end; ++ii ) {
      if( J[ ii ] < minJ ) {
         minJ = J[ ii ];
         minInd = ii;
      }
   }
   dip::uint maxInd = minInd + 1;
   for( ; ( maxInd < end ) && ( J[ maxInd ] == minJ ); ++maxInd ) {}
   return offset + ( static_cast< dfloat >( minInd ) + static_cast< dfloat >( maxInd )) / 2.0 * scale;
}

dfloat TriangleThreshold(
      Histogram const& in
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Histogram smoothIn = Smooth( in, 4 );
   Image const& hist = smoothIn.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   // Find the peak
   UnsignedArray maxCoords = MaximumPixel( hist );
   dip::uint maxElement = maxCoords[ 0 ];
   Histogram::CountType maxValue = data[ maxElement ];
   // Define: start, peak, stop positions in histogram
   VertexFloat left_bin{ 0.0, static_cast< dfloat >( data[ 0 ] ) };
   VertexFloat right_bin{ static_cast< dfloat >( nBins - 1 ), static_cast< dfloat >( data[ nBins - 1 ] ) };
   VertexFloat top_bin{ static_cast< dfloat >( maxElement ), static_cast< dfloat >( maxValue ) };
   // Find the location of the maximum distance to the triangle
   dip::uint bin = 0;
   dfloat maxDistance = 0;
   for( dip::uint ii = 1; ii < maxElement; ++ii ) {
      VertexFloat pos{ static_cast< dfloat >( ii ), static_cast< dfloat >( data[ ii ] ) };
      dfloat distance = TriangleHeight( left_bin, top_bin, pos );
      if( distance > maxDistance ) {
         maxDistance = distance;
         bin = ii;
      }
   }
   for( dip::uint ii = maxElement + 1; ii < nBins - 1; ++ii ) {
      VertexFloat pos{ static_cast< dfloat >( ii ), static_cast< dfloat >( data[ ii ] ) };
      dfloat distance = TriangleHeight( right_bin, top_bin, pos );
      if( distance > maxDistance ) {
         maxDistance = distance;
         bin = ii;
      }
   }
   return smoothIn.BinCenter( bin );
}

dfloat BackgroundThreshold(
      Histogram const& in,
      dfloat distance
) {
   DIP_THROW_IF( distance <= 0, E::INVALID_PARAMETER );
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Histogram smoothIn = Smooth( in, 4 );
   Image const& hist = smoothIn.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   // Find the peak
   UnsignedArray maxCoords = MaximumPixel( hist );
   dip::uint maxElement = maxCoords[ 0 ];
   Histogram::CountType maxValue = data[ maxElement ];
   dfloat threshold = smoothIn.BinCenter( maxElement );
   dfloat binSize = smoothIn.BinSize();
   // Is the peak on the left or right side of the histogram?
   bool rightPeak = maxElement > ( nBins / 2 );
   // Find the 50% height & the threshold
   if( rightPeak ) {
      dip::uint sigma = nBins - 1;
      for( ; sigma >= maxElement; --sigma ) {
         if( data[ sigma ] > maxValue / 2 ) {
            break;
         }
      }
      sigma -= maxElement;
      if( sigma == 0 ) {
         sigma = 1;
      }
      threshold -= static_cast< dfloat >( sigma ) * distance * binSize;
   } else {
      dip::uint sigma = 0;
      for( ; sigma <= maxElement; ++sigma ) {
         if( data[ sigma ] > maxValue / 2 ) {
            break;
         }
      }
      sigma = maxElement - sigma;
      if( sigma == 0 ) {
         sigma = 1;
      }
      threshold += static_cast< dfloat >( sigma ) * distance * binSize;
   }
   return threshold;
}

} // namespace dip
